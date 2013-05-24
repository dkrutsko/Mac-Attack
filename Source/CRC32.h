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

#ifndef CRC32_H
#define CRC32_H

#include "Types.h"



//----------------------------------------------------------------------------//
// Classes                                                                    //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> A utility class to compute CRC32 hashes. </summary>

class CRC32
{
public:
	// Constructors
	CRC32 (void);

public:
	// Functions
	void Add (uint32 length, const uint8* data);

public:
	// Properties
	uint32 Value;
};

#endif // CRC32_H