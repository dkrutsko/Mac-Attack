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

#include "Packet.h"
#include <cstring>
#include <netinet/in.h>



//----------------------------------------------------------------------------//
// Methods                                                             Packet //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Computes the total size of this packet. </summary>

uint32 Packet::ComputeSize (void) const
{
	return sizeof (Address) +
		   sizeof (Address) +
		   sizeof (uint16 ) +
		   sizeof (uint32 ) +
		   sizeof (uint32 ) +
		   sizeof (uint32 ) +
		   Msg.GetLength () +
		   sizeof (Address) * Addresses.size() +
		   sizeof (uint32 ) * Hashes.size();
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Serializes this packet and stores it in the parameters. </summary>
/// <remarks> This function returns false if the length is too small. </remarks>

bool Packet::Serialize (uint32 length, uint8* buffer) const
{
	// Is the buffer large enough
	if (length < ComputeSize())
		return false;

	// Get the size of the message and addresses
	uint32 messageLength = Msg.GetLength ();
	uint32 addressLength = Addresses.size();
	uint32 hashesLength  = Hashes.size();

	// Copy the source and target address to the buffer
	memcpy (buffer, Target.Data, sizeof (Address)); buffer += sizeof (Address);
	memcpy (buffer, Source.Data, sizeof (Address)); buffer += sizeof (Address);

	// Copy the IP Type to the buffer
	memcpy (buffer, &IPType, sizeof (uint16)); buffer += sizeof (uint16);

	// Copy the size of the message, addresses and hashes to the buffer
	memcpy (buffer, &messageLength, sizeof (uint32)); buffer += sizeof (uint32);
	memcpy (buffer, &addressLength, sizeof (uint32)); buffer += sizeof (uint32);
	memcpy (buffer, &hashesLength , sizeof (uint32)); buffer += sizeof (uint32);

	// Copy the message to the buffer
	memcpy (buffer, Msg.GetData(), messageLength); buffer += messageLength;

	// Copy every address to the buffer
	for (std::list<Address>::const_iterator i = Addresses.begin(); i != Addresses.end(); ++i)
	{
		memcpy (buffer, (*i).Data, sizeof (Address)); buffer += sizeof (Address);
	}

	// Copy every hash value to the buffer
	for (std::list<uint32>::const_iterator i = Hashes.begin(); i != Hashes.end(); ++i)
	{
		memcpy (buffer, &(*i), sizeof (uint32)); buffer += sizeof (uint32);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Deserializes the specified data into this packet. </summary>
/// <remarks> Any previous packet information will be destroyed. </remarks>

bool Packet::Deserialize (uint32 length, const uint8* buffer)
{
	// Delete any previous information
	Msg.Destroy(); Addresses.clear(); Hashes.clear();

	uint32 messageLength;
	uint32 addressLength;
	uint32 hashesLength;

	// Retrieve the source and target address from the buffer
	memcpy (Target.Data, buffer, sizeof (Address)); buffer += sizeof (Address);
	memcpy (Source.Data, buffer, sizeof (Address)); buffer += sizeof (Address);

	// Retrieve the IP Type from the buffer
	memcpy (&IPType, buffer, sizeof (uint16)); buffer += sizeof (uint16);

	// Check the IP Type
	if (IPType != htons (TYPE_BEACON) &&
		IPType != htons (TYPE_MESSAGE))
		return false;

	// Retrieve the size of the message, addresses and hashes from the buffer
	memcpy (&messageLength, buffer, sizeof (uint32)); buffer += sizeof (uint32);
	memcpy (&addressLength, buffer, sizeof (uint32)); buffer += sizeof (uint32);
	memcpy (&hashesLength , buffer, sizeof (uint32)); buffer += sizeof (uint32);

	// Retrieve the message from the buffer
	Msg.Create (messageLength);
	memcpy (Msg.GetData(), buffer, messageLength); buffer += messageLength;

	// Retrieve every address from the buffer
	Address address;
	for (uint32 i = 0; i < addressLength; ++i)
	{
		memcpy (address.Data, buffer, sizeof (Address)); buffer += sizeof (Address);
		Addresses.push_back (address);
	}

	// Retrieve every hash from the buffer
	uint32 hash;
	for (uint32 i = 0; i < hashesLength; ++i)
	{
		memcpy (&hash, buffer, sizeof (uint32)); buffer += sizeof (uint32);
		Hashes.push_back (hash);
	}

	return true;
}
