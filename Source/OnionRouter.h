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

#ifndef ONION_ROUTER_H
#define ONION_ROUTER_H

#include "Packet.h"
#include "Address.h"
#include "Message.h"
#include "Identity.h"

#include <list>
#include <pthread.h>

#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include <linux/if.h>
#include <sys/ioctl.h>



//----------------------------------------------------------------------------//
// Classes                                                                    //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Represents a single ORP instance. </summary>

class OnionRouter
{
	friend void* SendThread (void* parameters);
	friend void* RecvThread (void* parameters);

public:
	////////////////////////////////////////////////////////////////////////////////
	/// <summary> List of possible errors. </summary>

	enum Error
	{
		ERROR_NONE = 0,
		ERROR_INVALID_ID,
		ERROR_OPEN_SOCK,
		ERROR_GET_IFINDEX,
		ERROR_GET_ADDRESS,
		ERROR_GET_MTU,
		ERROR_ADD_PROM,
		ERROR_BIND_SOCK,
	};

public:
	////////////////////////////////////////////////////////////////////////////////
	/// <summary> Represents a single node. </summary>

	class Node
	{
	public:
		// Constructors
		 Node (void) { rsa_init (&Idnt, RSA_PKCS_V15, 0);	}
		~Node (void) { rsa_free (&Idnt);					}

	public:
		// Properties
		Address		Addr;			// Node address
		rsa_context	Idnt;			// Node identity

		bool		Arrived;		// Has arrived
		int8		Recorded;		// Last recorded

		// List of addresses in path
		std::list<Address> Addresses;
	};

public:
	// Constructors
	 OnionRouter					(void);
	~OnionRouter					(void);

public:
	// Methods
	Error			Create			(const std::string& interface, Identity* identity);
	void			Destroy			(void);

	void			Start			(void);
	void			Stop			(void);
	bool			IsActive		(void) const;

	bool			Send			(const Address& destination, const Message& message);
	Message*		Receive			(void);
	void			Flush			(void);

	void			Lock			(void);
	void			Unlock			(void);

	void			ReadIgnoreList	(const std::string& filename);

public:
	// Static
	static std::string ErrorString	(Error error);

private:
	// Internal
	bool			EncryptLayered	(const Address& destination,
									 const Message& input, Packet& packet);

	void			ProcessMessage	(      Packet& packet);
	void			ProcessBeacon	(const Packet& packet);
	void			UpdateNetwork	(void);

	void			CopyAddressPath	(Node* node, const Packet& packet);

public:
	// Properties
	std::list<Node*> Network;		// List of network nodes
		// Call lock/unlock before accessing this variable

private:
	// Fields
	rsa_context		mAuthority;		// Authority public key

	Identity*		mIdentity;		// Identity to use
	Address			mAddress;		// Local MAC address

	int32			mMTU;			// Socket MTU
	int32			mSocketID;		// Socket descriptor
	int32			mIfIndex;		// Interface index

	sockaddr_ll		mDest;			// Destination
	uint8			mDestLength;	// Destination length

	pthread_t		mSendThread;	// Send thread ID
	pthread_t		mRecvThread;	// Recv thread ID
	pthread_mutex_t	mMutex;			// Synchronization
	volatile bool	mActive;		// Currently active

	std::list<Message*> mMessages;	// List of queued messages
	std::list<Address > mIgnore;	// List of addresses to ignore
};

#endif // ONION_ROUTER_H
