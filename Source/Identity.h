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

#ifndef IDENTITY_H
#define IDENTITY_H

#include "Types.h"
#include <string>

#include "polarssl/rsa.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/entropy.h"



//----------------------------------------------------------------------------//
// constants                                                                  //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Defines the RSA exponent to use for all identities. </summary>

#define EXPONENT 65537



//----------------------------------------------------------------------------//
// Classes                                                                    //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Represents a single identity. </summary>

class Identity
{
public:
	////////////////////////////////////////////////////////////////////////////////
	/// <summary> List of possible errors. </summary>

	enum Error
	{
		ERROR_NONE = 0,
		ERROR_FILE_OPEN,
		ERROR_CTR_DRBG_INIT,
		ERROR_RSA_GEN_KEY,
		ERROR_MPI_READ_FILE,
		ERROR_MPI_WRITE_FILE,
		ERROR_SIGN_KEY_LENGTH,
		ERROR_RSA_PRIVATE,
	};

public:
	// Constructors
	 Identity				(void);
	~Identity				(void);

private:
	Identity				(const Identity& identity) { }

public:
	// Methods
	Error 		Create		(uint16 length);

	Error		Load		(const std::string& filename);
	Error		Save		(const std::string& filename) const;

	Error		Sign		(Identity& identity);

public:
	// Static
	static std::string ErrorString (Error error);

public:
	// Properties
	uint32		SignLength;	// Length of the signature

	mpi			AuthKey;	// CA public key
	mpi			SignKey;	// Encrypted token

	rsa_context	RsaState;	// Public and private key
};

#endif // IDENTITY_H
