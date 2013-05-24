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

#include <cstdio>
#include "Identity.h"



//----------------------------------------------------------------------------//
// Constructors                                                      Identity //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Creates a new uninitialized identity. </summary>

Identity::Identity (void)
{
	SignLength = 0;

	mpi_init (&AuthKey);
	mpi_init (&SignKey);

	rsa_init (&RsaState, RSA_PKCS_V15, 0);
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Deletes this identity and deallocates all data. </summary>

Identity::~Identity (void)
{
	mpi_free (&AuthKey);
	mpi_free (&SignKey);

	rsa_free (&RsaState);
}



//----------------------------------------------------------------------------//
// Methods                                                           Identity //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Create a new unsigned identity token. </summary>
/// <remarks> This function Destroys the previous identity. </remarks>

Identity::Error Identity::Create (uint16 length)
{
	entropy_context  entropy;
	ctr_drbg_context ctr_drbg;

	// Create an entropy object
	entropy_init (&entropy);

	// Initialize the random number generator
	if (ctr_drbg_init (&ctr_drbg, entropy_func, &entropy, null, 0) != 0)
		return ERROR_CTR_DRBG_INIT;

	// Generate an RSA key
	if (rsa_gen_key (&RsaState, ctr_drbg_random, &ctr_drbg, length, EXPONENT) != 0)
		return ERROR_RSA_GEN_KEY;

	SignLength = 0;
	return ERROR_NONE;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Loads an identity token from a file. </summary>
/// <remarks> This function Destroys the previous identity. </remarks>

Identity::Error Identity::Load (const std::string& filename)
{
	// Attempt to open the file
	FILE* file = fopen (filename.c_str(), "rb");
	if (file == null) return ERROR_FILE_OPEN;

	// Read the public and private keys
	if (mpi_read_file (&RsaState.N , 16, file) != 0 ||
		mpi_read_file (&RsaState.D , 16, file) != 0 ||
		mpi_read_file (&RsaState.P , 16, file) != 0 ||
		mpi_read_file (&RsaState.Q , 16, file) != 0 ||
		mpi_read_file (&RsaState.DP, 16, file) != 0 ||
		mpi_read_file (&RsaState.DQ, 16, file) != 0 ||
		mpi_read_file (&RsaState.QP, 16, file) != 0)
		{ fclose (file); return ERROR_MPI_READ_FILE; }

	// Read the signature information
	SignLength = 0;
	if (mpi_read_file (&AuthKey, 16, file) == 0 &&
		mpi_read_file (&SignKey, 16, file) == 0)
		SignLength = (mpi_msb (&AuthKey) + 7) >> 3;

	// Calculate the RSA length and set exponent
	RsaState.len = (mpi_msb (&RsaState.N) + 7) >> 3;
	mpi_lset (&RsaState.E, EXPONENT);

	fclose (file);
	return ERROR_NONE;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Saves an identity token to a file. </summary>

Identity::Error Identity::Save (const std::string& filename) const
{
	// Attempt to open the file
	FILE* file = fopen (filename.c_str(), "wb+");
	if (file == null) return ERROR_FILE_OPEN;

	// Write the public and private keys
	if (mpi_write_file (null, &RsaState.N , 16, file) != 0 ||
		mpi_write_file (null, &RsaState.D , 16, file) != 0 ||
		mpi_write_file (null, &RsaState.P , 16, file) != 0 ||
		mpi_write_file (null, &RsaState.Q , 16, file) != 0 ||
		mpi_write_file (null, &RsaState.DP, 16, file) != 0 ||
		mpi_write_file (null, &RsaState.DQ, 16, file) != 0 ||
		mpi_write_file (null, &RsaState.QP, 16, file) != 0)
		{ fclose (file); return ERROR_MPI_WRITE_FILE; }

	if (SignLength != 0)
	{
		// Write the signature information
		if (mpi_write_file (null, &AuthKey, 16, file) != 0 ||
			mpi_write_file (null, &SignKey, 16, file) != 0)
			{ fclose (file); return ERROR_MPI_WRITE_FILE; }
	}

	fclose (file);
	return ERROR_NONE;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> Signs the specified unsigned identity token. </summary>

Identity::Error Identity::Sign (Identity& identity)
{
	// Check the length of the keys
	if (RsaState.len <= identity.RsaState.len)
		return ERROR_SIGN_KEY_LENGTH;

	// Create a large enough buffer
	uint8* buffer = new uint8[RsaState.len];
	memset (buffer, 0, RsaState.len);

	// Copy the public key to the end of the buffer
	uint8* d = buffer + (RsaState.len - identity.RsaState.len);
	mpi_write_binary (&identity.RsaState.N, d, identity.RsaState.len);

	// Encrypt the public key buffer
	if (rsa_private (&RsaState, buffer, buffer) != 0)
		{ delete[] buffer; return ERROR_RSA_PRIVATE; }

	// Apply the signatures
	identity.SignLength = RsaState.len;
	mpi_read_binary (&identity.SignKey, buffer, RsaState.len);
	mpi_copy (&identity.AuthKey, &RsaState.N);

	delete[] buffer;
	return ERROR_NONE;
}



//----------------------------------------------------------------------------//
// Static                                                            Identity //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Returns the string representation of a specified error. </summary>

std::string Identity::ErrorString (Error error)
{
	switch (error)
	{
		case ERROR_NONE				: return "";
		case ERROR_FILE_OPEN		: return "Failed to open file";
		case ERROR_CTR_DRBG_INIT	: return "Failed to initialize CTR_DRBG";
		case ERROR_RSA_GEN_KEY		: return "Failed to generate an RSA key";
		case ERROR_MPI_READ_FILE	: return "Failed to read an MPI from file";
		case ERROR_MPI_WRITE_FILE	: return "Failed to write and MPI to file";
		case ERROR_SIGN_KEY_LENGTH	: return "The signing key must be larger than the key being signed";
		case ERROR_RSA_PRIVATE		: return "Failed to perform the RSA private operation";
		default						: return "Unknown error occurred";
	}
}
