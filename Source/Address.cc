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

#include "Address.h"



//----------------------------------------------------------------------------//
// Constants                                                          Address //
//----------------------------------------------------------------------------//

const Address Address::Null      = Address (  0,   0,   0,   0,   0,   0);
const Address Address::Broadcast = Address (255, 255, 255, 255, 255, 255);



//----------------------------------------------------------------------------//
// Constructors                                                       Address //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a new uninitialized address. </summary>

Address::Address (void) { }

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a copy of the specified address. </summary>

Address::Address (const Address& address)
{
	Data[0] = address.Data[0];
	Data[1] = address.Data[1];
	Data[2] = address.Data[2];
	Data[3] = address.Data[3];
	Data[4] = address.Data[4];
	Data[5] = address.Data[5];
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a new address initialized to the specified values. </summary>

Address::Address (uint8 a, uint8 b, uint8 c, uint8 d, uint8 e, uint8 f)
{
	Data[0] = a;
	Data[1] = b;
	Data[2] = c;
	Data[3] = d;
	Data[4] = e;
	Data[5] = f;
}



//----------------------------------------------------------------------------//
// Functions                                                          Address //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Returns the string representation of this error. </summary>

std::string Address::ToString (void) const
{
	char result[32];

	sprintf (result, "%02X:%02X:%02X:%02X:%02X:%02X",
		Data[0], Data[1], Data[2], Data[3], Data[4], Data[5]);

	return result;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Sets this address to that of the string. </summary>

void Address::FromString (std::string address)
{
	// Get the string pointer
	const char* a = address.c_str();

	// Reset the address
	Data[0] = 0; Data[1] = 0;
	Data[2] = 0; Data[3] = 0;
	Data[4] = 0; Data[5] = 0;

	// Value needs to be multiplied
	bool highValue = true;

	// Loop through the string
	for (uint32 i = 0, j = 0; i < address.length() && j < Length; ++i)
	{
		// Get the high value
		  if (a[i] >= '0' && a[i] <= '9') Data[j] += a[i] - '0';
		elif (a[i] >= 'a' && a[i] <= 'f') Data[j] += a[i] - 'a' + 10;
		elif (a[i] >= 'A' && a[i] <= 'F') Data[j] += a[i] - 'A' + 10;
		else continue;

		if (highValue)
		{
			Data[j] *= 16;
			highValue = false;
		}

		else
		{
			++j;
			highValue = true;
		}
	}
}



//----------------------------------------------------------------------------//
// Operators                                                          Address //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Tests two addresses for equality. </summary>

bool operator == (const Address& value1, const Address& value2)
{
	return value1.Data[0] == value2.Data[0] &&
		   value1.Data[1] == value2.Data[1] &&
		   value1.Data[2] == value2.Data[2] &&
		   value1.Data[3] == value2.Data[3] &&
		   value1.Data[4] == value2.Data[4] &&
		   value1.Data[5] == value2.Data[5];
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Tests two addresses for inequality. </summary>

bool operator != (const Address& value1, const Address& value2)
{
	return value1.Data[0] != value2.Data[0] ||
		   value1.Data[1] != value2.Data[1] ||
		   value1.Data[2] != value2.Data[2] ||
		   value1.Data[3] != value2.Data[3] ||
		   value1.Data[4] != value2.Data[4] ||
		   value1.Data[5] != value2.Data[5];
}
