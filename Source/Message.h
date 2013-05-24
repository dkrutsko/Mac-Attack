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

#ifndef MESSAGE_H
#define MESSAGE_H

#include "Types.h"



//----------------------------------------------------------------------------//
// Classes                                                                    //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Represents a single message. </summary>

class Message
{
public:
	// Constructors
	 Message				(void);
	~Message				(void);
	 Message				(const Message& message);

public:
	// Methods
	void	Create			(uint32 length);
	void	Destroy			(void);

	uint32	GetLength		(void) const;
	uint8*	GetData			(void) const;

public:
	// Operators
	Message& operator =		(const Message& message);

private:
	// Fields
	uint32	mLength;		// Message data length
	uint8*	mData;			// Message data
};

#endif // MESSAGE_H
