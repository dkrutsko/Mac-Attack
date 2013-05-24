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

#ifndef TYPES_H
#define TYPES_H



//----------------------------------------------------------------------------//
// Keywords                                                                   //
//----------------------------------------------------------------------------//

#define null				0				// Null pointer
#define forever				while (true)	// Infinite loop
#define elif				else if			// Else if



//----------------------------------------------------------------------------//
// Types                                                                      //
//----------------------------------------------------------------------------//

typedef signed char			int8;			// Signed  8-bit integer
typedef signed short		int16;			// Signed 16-bit integer
typedef signed int			int32;			// Signed 32-bit integer
typedef signed long long	int64;			// Signed 64-bit integer

typedef unsigned char		uint8;			// Unsigned  8-bit integer
typedef unsigned short		uint16;			// Unsigned 16-bit integer
typedef unsigned int		uint32;			// Unsigned 32-bit integer
typedef unsigned long long	uint64;			// Unsigned 64-bit integer

typedef float				real32;			// 32-bit  real value
typedef double				real64;			// 64-bit  real value

#endif // TYPES_H
