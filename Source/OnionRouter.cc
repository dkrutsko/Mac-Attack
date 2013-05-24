////////////////////////////////////////////////////////////////////////////////
// -------------------------------------------------------------------------- //
//                                                                            //
//                          Copyright (C) 2012-2013                           //
//                            github.com/dkrutsko                             //
//                            github.com/Harrold                              //
//                            github.com/AbsMechanik                          //
//                                                                            //
//                        See LICENSE.md for copyright                        //
//                                                                            //
// -------------------------------------------------------------------------- //
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------//
// Prefaces                                                                   //
//----------------------------------------------------------------------------//

#include "CRC32.h"
#include "OnionRouter.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

using std::list;
using std::string;



//----------------------------------------------------------------------------//
// Types                                                                      //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Maximum number of messages to store. </summary>

#define MAX_MESSAGES 128

////////////////////////////////////////////////////////////////////////////////
/// <summary> Maximum last recorded value allowed. </summary>
/// <remarks> This value is inclusive [-1, max]. </remarks>

#define MAX_RECORDED 5



//----------------------------------------------------------------------------//
// Threading                                                                  //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Thread that handles sending beacon packets. </summary>

void* SendThread (void* parameters)
{
	// Retrieve the OnionRouter instance
	OnionRouter* router = (OnionRouter*) parameters;

	// Create the packet
	Packet packet;
	packet.Target = Address::Broadcast;
	packet.Source = router->mAddress;
	packet.IPType = htons (Packet::TYPE_BEACON);

	// Copy the public token into the message
	packet.Msg.Create (router->mIdentity->SignLength);
	mpi_write_binary (&router->mIdentity->SignKey,
		packet.Msg.GetData(), router->mIdentity->SignLength);

	// Serialize packet and prepare for sending
	uint32 bufferLength = packet.ComputeSize();
	uint8* buffer = new uint8 [bufferLength];
	packet.Serialize (bufferLength, buffer);

	// Enter the send loop
	uint32 elapsed = 10000000;
	while (router->mActive)
	{
		if (elapsed > 5000000)
		{
			// Reset timer
			elapsed = 0;

			// Send message
			sendto (router->mSocketID, buffer, bufferLength, 0,
				(sockaddr*) &router->mDest, router->mDestLength);
		}

		// Sleep for 100 ms
		usleep (10000);
		elapsed += 10000;
	}

	delete[] buffer;
	return null;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Thread that handles receiving beacon packets. </summary>

void* RecvThread (void* parameters)
{
	// Retrieve the OnionRouter instance
	OnionRouter* router = (OnionRouter*) parameters;

	// Create a large enough buffer
	uint8* data = new uint8 [router->mMTU];
	Packet packet;

	// Enter the receive loop
	uint32 elapsed = 0;
	while (router->mActive)
	{
		// Receive current packet
		if (recvfrom (router->mSocketID, data,
			router->mMTU, MSG_DONTWAIT, null, null) > 0 &&
			packet.Deserialize (router->mMTU, data))
		{
			// Process the packet as a message
			if (packet.IPType == htons (Packet::TYPE_MESSAGE))
				router->ProcessMessage (packet);

			// Process the packet as a beacon
			elif (packet.IPType == htons (Packet::TYPE_BEACON))
			{
				// Ignore if the address is the same as this node
				if (router->mAddress != packet.Source)
				{
					// Process beacon
					router->Lock();
					router->ProcessBeacon (packet);
					router->Unlock();

					// Broadcast beacon with new path
					packet.Addresses.push_back (router->mAddress);

					// Serialize packet and prepare for sending
					uint32 bufferLength = packet.ComputeSize();
					uint8* buffer = new uint8 [bufferLength];
					packet.Serialize (bufferLength, buffer);

					// Send the packet
					sendto (router->mSocketID, buffer, bufferLength,
						0, (sockaddr*) &router->mDest, router->mDestLength);

					delete[] buffer;
				}
			}
		}

		if (elapsed > 10000000)
		{
			// Update neightbor network
			router->Lock();
			router->UpdateNetwork();
			router->Unlock();

			// Reset timer
			elapsed = 0;
		}

		// Sleep for 100 ms
		usleep (10000);
		elapsed += 10000;
	}

	delete[] data;
	return null;
}



//----------------------------------------------------------------------------//
// Constructors                                                   OnionRouter //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a new uninitialized ORP. </summary>

OnionRouter::OnionRouter (void)
{
	mIdentity = null;
	mActive   = false;
	mSocketID = -1;

	pthread_mutex_init (&mMutex, null);
	rsa_init (&mAuthority, RSA_PKCS_V15, 0);
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Deletes the ORP and deallocates all data. </summary>
/// <remarks> This function makes a call to Destroy the ORP. </remarks>

OnionRouter::~OnionRouter (void)
{
	Destroy();
	pthread_mutex_destroy (&mMutex);
	rsa_free (&mAuthority);
}



//----------------------------------------------------------------------------//
// Methods                                                        OnionRouter //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Initializes the ORP using the specified interface. </summary>
/// <remarks> This function Destroys any previous ORP instance. </remarks>

OnionRouter::Error OnionRouter::Create (const string& interface, Identity* identity)
{
	// Destroy any previous instance
	Destroy();

	// Check for a valid identity
	if (identity == null || identity->SignLength == 0)
		return ERROR_INVALID_ID;

	// Save the identity
	mIdentity = identity;

	// Cache the authority public key
	mAuthority.len = mIdentity->SignLength;
	mpi_copy (&mAuthority.N, &mIdentity->AuthKey);
	mpi_lset (&mAuthority.E, EXPONENT);

	// Create device level socket
	mSocketID = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
		// PF_PACKET - Packet interface on device level
		// SOCK_RAW  - Raw packets including link level header
		// ETH_P_ALL - All frames will be received

	if (mSocketID < 0)
		return ERROR_OPEN_SOCK;

	// Fetch interface information
	ifreq ifr;

	// Copy the specified interface name
	strcpy (ifr.ifr_name, interface.c_str());

	// Retrieve the interface index
	if (ioctl (mSocketID, SIOGIFINDEX, &ifr) < 0)
		return ERROR_GET_IFINDEX;

	mIfIndex = ifr.ifr_ifindex;

	// Retrieve the hardware address
	if (ioctl (mSocketID, SIOCGIFHWADDR, &ifr) < 0)
		return ERROR_GET_ADDRESS;

	memcpy (&mAddress.Data, &ifr.ifr_hwaddr.sa_data, Address::Length);

	// Retrieve the maximum transmission unit
	if (ioctl (mSocketID, SIOCGIFMTU, &ifr) < 0)
		return ERROR_GET_MTU;

	mMTU = ifr.ifr_mtu;

	// Add promiscuous mode
	packet_mreq mr;
	memset (&mr, 0, sizeof (mr));

	mr.mr_ifindex = mIfIndex;
	mr.mr_type    = PACKET_MR_PROMISC;

	if (setsockopt (mSocketID, SOL_PACKET,
		PACKET_ADD_MEMBERSHIP, (char*) &mr, sizeof (mr)) < 0)
		return ERROR_ADD_PROM;

	// Bind the socket to the interface
	sockaddr_ll sll;
	memset (&sll, 0, sizeof (sll));

	sll.sll_family   = AF_PACKET;
	sll.sll_ifindex  = mIfIndex;
	sll.sll_protocol = htons (ETH_P_ALL);

	if (bind (mSocketID, (sockaddr*) &sll, sizeof (sll)) < 0)
		return ERROR_BIND_SOCK;

	// Create a destination packet
	mDestLength = sizeof (mDest);
	memset (&mDest, 0, mDestLength);

	mDest.sll_family  = AF_PACKET;
	mDest.sll_pkttype = PACKET_BROADCAST;
	mDest.sll_ifindex = mIfIndex;

	mDest.sll_halen = Address::Length;
	memset (mDest.sll_addr, 255, Address::Length);

	return ERROR_NONE;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Destroys the ORP and deallocates all data. </summary>
/// <remarks> This function makes a call to Stop the ORP. </remarks>

void OnionRouter::Destroy (void)
{
	// Close the socket
	if (mSocketID != -1)
	{
		Stop();
		close (mSocketID);

		mIdentity = null;
		mSocketID = -1;
	}
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Starts the onion routing protocol. </summary>
/// <remarks> This function does not block. </remarks>

void OnionRouter::Start (void)
{
	if (!mActive)
	{
		// Create thread
		mActive = true;
		pthread_create (&mSendThread, null, SendThread, this);
		pthread_create (&mRecvThread, null, RecvThread, this);
	}
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Stops the onion routing protocol. </summary>
/// <remarks> This function blocks until all threads have been joined. </remarks>

void OnionRouter::Stop (void)
{
	if (mActive)
	{
		// Join threads
		mActive = false;
		pthread_join (mSendThread, null);
		pthread_join (mRecvThread, null);

		// Clear messages
		Flush();

		// Clear network
		for (list<Node*>::iterator i = Network.
			begin(); i != Network.end(); ++i)
			delete *i;

		Network.clear();
	}
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Returns whether or not the onion router is running. </summary>

bool OnionRouter::IsActive (void) const
{
	return mActive;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Sends the specified message to the specified address. </summary>

bool OnionRouter::Send (const Address& destination, const Message& message)
{
	// Create the packet
	Packet packet;
	packet.Target = Address::Broadcast;
	packet.Source = mAddress;
	packet.IPType = htons (Packet::TYPE_MESSAGE);

	Lock();
	bool result = EncryptLayered
		(destination, message, packet);
	Unlock();

	// Destination is not found
	if (!result) return false;

	// Serialize packet and prepare for sending
	uint32 bufferLength = packet.ComputeSize();
	uint8* buffer = new uint8 [bufferLength];
	packet.Serialize (bufferLength, buffer);

	// Send the packet
	sendto (mSocketID, buffer, bufferLength,
		0, (sockaddr*) &mDest, mDestLength);

	delete[] buffer;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Receive any messages currently on the stack. </summary>
/// <remarks> Returns null if there are currently no messages. </remarks>

Message* OnionRouter::Receive (void)
{
	if (mMessages.size() > 0)
	{
		// Return the oldest message
		Message* result = mMessages.front();
		mMessages.pop_front(); return result;
	}

	return null;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Removes all messages on the stack. </summary>

void OnionRouter::Flush (void)
{
	for (list<Message*>::iterator i = mMessages.
		begin(); i != mMessages.end(); ++i) delete *i;

	mMessages.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Locks access to the node network list. </summary>

void OnionRouter::Lock (void)
{
	pthread_mutex_lock (&mMutex);
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Releases access to the node network list. </summary>

void OnionRouter::Unlock (void)
{
	pthread_mutex_unlock (&mMutex);
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Reads the ignore list from a file. </summary>

void OnionRouter::ReadIgnoreList (const string& filename)
{
	// Ignore if active
	if (mActive) return;

	// Clear any previous list
	mIgnore.clear();

	// Attempt to open the file
	FILE* file = fopen (filename.c_str(), "rb");
	if (file == null) return;

	char line[80];

	// Read addresses line by line
	while (fgets (line, 80, file))
	{
		Address address = Address::Null;
		address.FromString (line);
		mIgnore.push_back (address);
	}

	// Close the file
	fclose (file);
}



//----------------------------------------------------------------------------//
// Static                                                         OnionRouter //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Returns the string representation of a specified error. </summary>

string OnionRouter::ErrorString (Error error)
{
	switch (error)
	{
		case ERROR_NONE			: return "";
		case ERROR_INVALID_ID	: return "The identity must be signed and valid";
		case ERROR_OPEN_SOCK	: return "Could not open socket, Try running with sudo";
		case ERROR_GET_IFINDEX	: return "Failed to retrieve the interface index";
		case ERROR_GET_ADDRESS	: return "Failed to retrieve the hardware address";
		case ERROR_GET_MTU		: return "Failed to retrieve the maximum transmission unit";
		case ERROR_ADD_PROM		: return "Failed to add the promiscuous mode";
		case ERROR_BIND_SOCK	: return "Failed to bind the socket to the interface";
		default					: return "Unknown error occurred";
	}
}



//----------------------------------------------------------------------------//
// Internal                                                       OnionRouter //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Applies layers of encryption based on the address path. </summary>

bool OnionRouter::EncryptLayered (const Address&
	destination, const Message& input, Packet& packet)
{
	CRC32 crc;

	// Find the node matching destination
	for (list<Node*>::iterator i = Network.
		begin(); i != Network.end(); ++i)
	{
		if ((*i)->Addr == destination)
		{
			// Create the output message
			packet.Msg.Create ((*i)->Idnt.len);

			// Make sure that the length of the input
			// does not exceed the length of the output
			if (input.GetLength() >= packet.Msg.GetLength())
				return false;

			// Reset the message contents to zero
			memset (packet.Msg.GetData(), 0, packet.Msg.GetLength());

			// Copy input to the end of output
			uint8* d = packet.Msg.GetData() + (packet.Msg.GetLength() - input.GetLength());
			memcpy (d, input.GetData(), input.GetLength());

			// Add the hash code of the message
			crc.Add (packet.Msg.GetLength(), packet.Msg.GetData());
			packet.Hashes.push_back (crc.Value);

			// Encrypt the message
			rsa_public (&(*i)->Idnt, packet.Msg.GetData(), packet.Msg.GetData());

			// Begin encrypting the message
			for (list<Address>::iterator j = (*i)->Addresses.
				begin(); j != (*i)->Addresses.end(); ++j)
			{
				bool status = false;

				// Find the node matching address
				for (list<Node*>::iterator k = Network.
					begin(); k != Network.end(); ++k)
				{
					if ((*k)->Addr == (*j))
					{
						// Add the hash code of the message
						crc.Value = 0;
						crc.Add (input.GetLength(), input.GetData());
						packet.Hashes.push_back (crc.Value);

						// Encrypt the message
						rsa_public (&(*k)->Idnt,
							packet.Msg.GetData(), packet.Msg.GetData());

						break;
					}
				}

				// There was a problem
				if (!status) return false;
			}

			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Processes the specified message. </summary>

void OnionRouter::ProcessMessage (Packet& packet)
{
	// Decrypt the message
	rsa_private (&mIdentity->RsaState, packet.Msg.GetData(), packet.Msg.GetData());

	// Verify that the data is correct
	CRC32 crc;
	crc.Add (packet.Msg.GetLength(), packet.Msg.GetData());

	// Hash is incorrect, ignore message
	if (crc.Value != packet.Hashes.back()) return;

	// Rebroadcast if the message is not the destination
	if (packet.Hashes.size() != 1)
	{
		// Broadcast message with new path
		packet.Hashes.pop_back();

		// Serialize packet and prepare for sending
		uint32 bufferLength = packet.ComputeSize();
		uint8* buffer = new uint8 [bufferLength];
		packet.Serialize (bufferLength, buffer);

		// Send the packet
		sendto (mSocketID, buffer, bufferLength,
			0, (sockaddr*) &mDest, mDestLength);

		delete[] buffer;
		return;
	}

	Message* message = new Message();

	// Strip the leading zeroes
	for (uint32 i = 0; i < packet.Msg.GetLength(); ++i)
	{
		if (packet.Msg.GetData()[i] != 0)
		{
			uint32 msgLength = packet.Msg.GetLength() - i;
			message->Create (msgLength);
			memcpy (message->GetData(), packet.Msg.GetData() + i, msgLength);
			break;
		}
	}

	// Add the message to the stack
	mMessages.push_back (message);

	// Delete old messages
	if (mMessages.size() >= MAX_MESSAGES)
	{
		delete mMessages.front();
		mMessages.pop_front();
	}
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Processes the specified beacon. </summary>

void OnionRouter::ProcessBeacon (const Packet& packet)
{
	// Ignore if part of ignore list
	for (list<Address>::iterator i = mIgnore.
		begin(); i != mIgnore.end(); ++i)
	{
		if ((*i) == packet.Source)
			return;
	}

	// Find the node matching packet source
	for (list<Node*>::iterator i = Network.
		begin(); i != Network.end(); ++i)
	{
		if ((*i)->Addr == packet.Source)
		{
			(*i)->Arrived = true;
			CopyAddressPath (*i, packet);
			return;
		}
	}

	// Decrypt the public key
	uint32 length = packet.Msg.GetLength();

	uint8* buffer = new uint8 [length];
	memcpy (buffer, packet.Msg.GetData(), length);

	if (rsa_public (&mAuthority, packet.Msg.GetData(), buffer) != 0)
		{ delete[] buffer; return; }

	// Add a new node
	Node* node = new Node;
	node->Arrived  = true;
	node->Recorded = -1;
	node->Addr     = packet.Source;

	// Copy public key information
	mpi_read_binary (&node->Idnt.N, buffer, length);
	node->Idnt.len = (mpi_msb (&node->Idnt.N) + 7) >> 3;
	mpi_lset (&node->Idnt.E, EXPONENT);

	// Check for key compatability
	if (node->Idnt.len != mIdentity->RsaState.len)
		{ delete[] buffer; return; }

	// Copy address path
	CopyAddressPath (node, packet);

	delete[] buffer;
	Network.push_back (node);
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Updates the neighbor table. </summary>

void OnionRouter::UpdateNetwork (void)
{
	list<Node*>::iterator i = Network.begin();
	while (i != Network.end())
	{
		// Check if we recieved a beacon
		if ((*i)->Arrived == false)
		{
			// Remove disconnected neighbors
			if (++(*i)->Recorded >= MAX_RECORDED)
			{
				// Deallocate and remove entry
				delete (*i);
				i = Network.erase(i);
			}

			else ++i;
		}

		else
		{
			// Reset arrival state
			(*i)->Arrived  = false;
			(*i)->Recorded = 0;
			++i;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Copies the packet's address path to the node. </summary>

void OnionRouter::CopyAddressPath (Node* node, const Packet& packet)
{
	// Copy the address path
	node->Addresses.clear();
	for (list<Address>::const_iterator iter = packet.
		Addresses.begin(); iter != packet.Addresses.end(); ++iter)
		node->Addresses.push_back (*iter);
}
