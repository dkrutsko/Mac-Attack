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

#ifndef PACKET_H
#define PACKET_H

#include "Address.h"
#include "Message.h"

#include <list>



//----------------------------------------------------------------------------//
// Classes                                                                    //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Represents a single packet. </summary>

class Packet
{
public:
	////////////////////////////////////////////////////////////////////////////////
	/// <summary> List of packet IP Types (Packet types). </summary>

	enum Type
	{
		TYPE_BEACON  = 0x3950,
		TYPE_MESSAGE = 0x3960,
	};

public:
	// Constructors
	 Packet (void) { }
	~Packet (void) { }

private:
	Packet (const Packet& packet);

public:
	// Methods
	uint32	ComputeSize	(void) const;

	bool	  Serialize	(uint32 length, uint8* buffer) const;
	bool	Deserialize	(uint32 length, const uint8* buffer);

public:
	// Properties
	Address	Target;		// Target address
	Address	Source;		// Source address
	uint16	IPType;		// IP Packet Type
	Message	Msg;		// Message data

	// List of addresses in path
	std::list<Address> Addresses;

	// List of hash values
	std::list<uint32> Hashes;
};

#endif // PACKET_H
