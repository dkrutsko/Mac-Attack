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

#include "Message.h"
#include <cstring>



//----------------------------------------------------------------------------//
// Constructors                                                       Message //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a new message with no content. </summary>

Message::Message (void)
{
	// Create a null message
	mLength = 0;
	mData   = null;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Deletes this message and deallocates all data. </summary>

Message::~Message (void)
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a copy of the specified message. </summary>

Message::Message (const Message& message)
{
	// Create a copy of the data
	mLength = message.mLength;
	mData   = message.mData;

	if (mLength != 0)
	{
		mData = new uint8[mLength];
		memcpy (mData, message.mData, mLength);
	}
}



//----------------------------------------------------------------------------//
// Methods                                                            Message //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a new message with the specified length. </summary>
/// <remarks> This function Destroys any previous message. </remarks>

void Message::Create (uint32 length)
{
	if (length > 0)
	{
		// Delete previous data
		Destroy();

		// Create message
		mLength = length;
		mData   = new uint8 [mLength];
	}
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Destroys this message and deallocates all data. </summary>

void Message::Destroy (void)
{
	// Delete the data
	if (mData != null)
	{
		delete[] mData;
		mLength = 0;
		mData = null;
	}
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Returns the length of this message. </summary>

uint32 Message::GetLength (void) const
{
	return mLength;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Returns a pointer to the message data, or null
///           if the message has not yet been created. </summary>

uint8* Message::GetData (void) const
{
	return mData;
}



//----------------------------------------------------------------------------//
// Operators                                                          Message //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> </summary>

Message& Message::operator = (const Message& message)
{
	// Handling of self assignment
	if (this == &message) return *this;

	// Destroy previous message
	Destroy();

	// Create a copy of the data
	mLength = message.mLength;
	mData   = message.mData;

	if (mLength != 0)
	{
		mData = new uint8[mLength];
		memcpy (mData, message.mData, mLength);
	}

	return *this;
}
