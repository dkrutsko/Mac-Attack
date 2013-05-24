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

#ifndef ADDRESS_H
#define ADDRESS_H

#include "Types.h"
#include <string>



//----------------------------------------------------------------------------//
// Classes                                                                    //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Represents an address type. </summary>

class Address
{
public:
	// Constants
	static const Address Null;		// 00:00:00:00:00:00
	static const Address Broadcast;	// FF:FF:FF:FF:FF:FF

	static const uint8 Length = 6;	// Length of an address

public:
	// Constructors
	Address (void);
	Address (const Address& address);

	Address (uint8 a, uint8 b,
			 uint8 c, uint8 d,
			 uint8 e, uint8 f);

public:
	// Functions
	std::string	ToString			(void) const;
	void		FromString			(std::string address);

public:
	// Properties
	uint8 Data[Length];				// Address data
};



//----------------------------------------------------------------------------//
// Operators                                                          Address //
//----------------------------------------------------------------------------//

bool operator == (const Address& value1, const Address& value2);
bool operator != (const Address& value1, const Address& value2);

#endif // ADDRESS_H
