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

#include "CRC32.h"
#include "OnionRouter.h"

#include <cstdio>
#include <csignal>
#include <cstdlib>



//----------------------------------------------------------------------------//
// Macros                                                                     //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Enables or disables printing messages in bold. </summary>

#define  ENABLE_BOLD printf ("\033[1m")
#define DISABLE_BOLD printf ("\033[0m")



//----------------------------------------------------------------------------//
// Functions                                                                  //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary>
///   Returns a pointer to the first occurrence of str2 in str1, or a
///   null pointer if str2 is not part of str1. The matching process is
///   case insensitive does not include the terminating null-characters.
/// </summary>
/// <remarks> This function only works on ASCII characters. </remarks>

static const char* FindString (const char* str1, const char* str2)
{
	const char* a;
	const char* b;

	for (; *str1 != 0; ++str1)
	{
		a = str1;
		b = str2;

		while ((*(a++) | 32) == (*(b++) | 32))
			if (*b == 0) return str1;
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// <summary> </summary>

static void JoinNetwork (OnionRouter& router)
{
	// Start the router
	router.Start();
	char command[512];

	system ("clear");
	ENABLE_BOLD; printf ("MAC ATTACK NETWORK TERMINAL\n\n"); DISABLE_BOLD;
	printf ("Enter \"Help\" for a list of usable commands\n");
	printf ("Enter \"Exit\" to exit the system\n\n");

	while (router.IsActive())
	{
		// Ask for a command
		printf ("Enter a command: ");
		scanf ("%s", command);

		// Send a message to an address
		if (FindString (command, "Send"))
		{
			// Ask for a destination
			printf ("Enter the destination: ");
			scanf ("%s", command);

			Address address = Address::Null;
			address.FromString (command);

			// Flush the input buffer
			int32 c;
			while ((c = getchar()) != '\n' && c != EOF);

			// Ask for a message
			printf ("Enter the message: ");
			fgets (command, 512, stdin);

			uint32 length = strlen (command);

			// Remove trailing newline, if any
			if (command[length - 1] == '\n')
				command[length - 1]  = '\0';

			// Build message
			Message message;
			message.Create (length);
			memcpy (message.GetData(), command, length);

			// Send the message
			if (router.Send (address, message))
				printf ("\nMessage sent to: %s\n\n", address.ToString().c_str());

			else printf ("\nFailed to send message\n\n");
		}

		// Receive the oldest message on the stack
		elif (FindString (command, "Recv") ||
			  FindString (command, "Receive"))
		{
			Message* message = router.Receive();

			if (message == null)
				printf ("\nNo new messages\n\n");

			else
			{
				printf ("\n%s\n\n", message->GetData());
				delete message;
			}
		}

		// Clears all the messages on the stack
		elif (FindString (command, "Flush"))
			router.Flush();

		// List all nodes in the network
		elif (FindString (command, "ls") ||
			  FindString (command, "List"))
		{
			router.Lock();
			printf ("\n");

			for (std::list<OnionRouter::Node*>::iterator i = router.
				Network.begin(); i != router.Network.end(); ++i)
			{
				printf ("Address: %s  KeyLength: %d  Arrived: %s  Recorded: %d\n",
					(*i)->Addr.ToString().c_str(), (uint32) (*i)->Idnt.len,
					(*i)->Arrived ? "True " : "False", (*i)->Recorded);
			}

			printf ("\n");
			router.Unlock();
		}

		// Clear the terminal window
		elif (FindString (command, "Cls") ||
			  FindString (command, "Clear"))
			system ("clear");

		// Leave the network and exit
		elif (FindString (command, "Quit") ||
			  FindString (command, "Exit"))
			  router.Stop();

		// Print the documentation
		elif (FindString (command, "Help"))
		{
			ENABLE_BOLD; printf ("\nSend\t"); DISABLE_BOLD;
			printf ("- Sends a message to the specified host\n");
			ENABLE_BOLD; printf ("Recv\t"); DISABLE_BOLD;
			printf ("- Receives the oldest message (if available)\n");
			ENABLE_BOLD; printf ("Flush\t"); DISABLE_BOLD;
			printf ("- Delete all pending messages\n");
			ENABLE_BOLD; printf ("List\t"); DISABLE_BOLD;
			printf ("- Lists all nodes in this network\n");
			ENABLE_BOLD; printf ("Clear\t"); DISABLE_BOLD;
			printf ("- Clears this terminal window\n");
			ENABLE_BOLD; printf ("Exit\t"); DISABLE_BOLD;
			printf ("- Leaves this network and closes the application\n\n");
		}

		else printf ("Command unrecognized, enter "
			"\"help\" for a full list of commands\n");
	}

	// Stop the router
	router.Stop();
}



//----------------------------------------------------------------------------//
// Main                                                                       //
//----------------------------------------------------------------------------//

////////////////////////////////////////////////////////////////////////////////
/// <summary> Main execution point for this application. </summary>
/// <param name="argc"> Number of arguments in the command line. </param>
/// <param name="argv"> Arguments from the command line. </param>
/// <returns> Zero for success, error code for failure. </returns>

int main (int argc, char** argv)
{
	// Evaluate the command being used
	// Create an unsigned identity token
	if (argc >= 4 && FindString (argv[1], "Create"))
	{
		Identity identity;
		uint16 length = (uint16) atoi (argv[2]);

		if (length >= 4088)
			printf ("Length must be less than 4088\n");

		else for (int i = 3; i < argc; ++i)
		{
			// Create an identity
			if (identity.Create (length) != 0)
				printf ("Unable to create identity\n");

			// Save the identity
			elif (identity.Save (argv[i]) != 0)
				printf ("Unable to save identity\n");
		}
	}

	// Get information about the indentity
	elif (argc >= 3 && FindString (argv[1], "Info"))
	{
		Identity identity;

		if (identity.Load (argv[2]) != 0)
			printf ("Unable to load identity\n");

		else
		{
			printf ("\n     Signed: %s\n", identity.SignLength > 0 ? "TRUE" : "FALSE");
			printf ("Sign Length: %u Bytes\n", identity.SignLength);
			printf (" Key Length: %u Bytes\n\n", (uint32) identity.RsaState.len);
		}
	}

	// Sign an unsigned identity token
	elif (argc >= 4 && FindString (argv[1], "Sign"))
	{
		Identity identity;
		Identity authority;

		// Load the authority
		if (authority.Load (argv[2]) != 0)
			printf ("Unable to load authority\n");

		else for (int i = 3; i < argc; ++i)
		{
			// Load the identity
			if (identity.Load (argv[i]) != 0)
				printf ("Unable to load identity\n");

			// Sign the identity
			elif (authority.Sign (identity) != 0)
				printf ("Unable to sign identity\n");

			// Save the identity
			elif (identity.Save (argv[i]) != 0)
				printf ("Unable to save identity\n");
		}
	}

	// Start the Onion Routing Protocol
	elif (argc >= 4 && FindString (argv[1], "Join"))
	{
		Identity identity;
		OnionRouter router;

		// Load the identity
		if (identity.Load (argv[3]) != 0)
			printf ("Unable to load identity\n");

		elif (identity.SignLength == 0)
			printf ("The identity must be signed\n");

		else
		{
			// Create an onion router
			OnionRouter::Error error = router.Create (argv[2], &identity);
			if (error != OnionRouter::ERROR_NONE)
				printf ("%s\n", OnionRouter::ErrorString (error).c_str());

			else
			{
				// Read the ignore list file (if any)
				if (argc >= 5)
					router.ReadIgnoreList (argv[4]);

				// Join the network
				JoinNetwork (router);
			}
		}
	}

	// Print the documentation
	elif (argc >= 2 && FindString (argv[1], "Help"))
	{
		ENABLE_BOLD; printf ("\nMAC ATTACK\n"); DISABLE_BOLD; printf ("----------\n\n");

		ENABLE_BOLD; printf ("DESCRIPTION\n"); DISABLE_BOLD;
		printf ("  This  application  implements the  onion routing protocol  over a\n");
		printf ("  wireless Ad-Hoc network using a  peer-to-peer architecture.  Each\n");
		printf ("  instance  of this  application is a node in  the network  that is\n");
		printf ("  able to discover other nodes and be discovered through a neighbor\n");
		printf ("  discovery  protocol.  Each node must  have an  identity  that  is\n");
		printf ("  compatible with other nodes in the network,  that is, an identity\n");
		printf ("  must be  signed by a common node  called the certification  node.\n");
		printf ("  Identities  can be  created  and signed  using this  application.\n\n");

		ENABLE_BOLD; printf ("COMMANDS\n"); DISABLE_BOLD;
		printf ("  $ MacAttack -Create [Key Length] [Filename ...]\n");
		printf ("  $ MacAttack -Info   [Identity]\n");
		printf ("  $ MacAttack -Sign   [Authority] [Filename ...]\n");
		printf ("  $ MacAttack -Join   [Interface] [Identity] (Ignore List)\n\n");

		printf ("   - Wildcards are not supported\n\n");

		ENABLE_BOLD; printf ("AUTHORS\n"); DISABLE_BOLD;
		printf ("  github.com/dkrutsko   \n");
		printf ("  github.com/Harrold    \n");
		printf ("  github.com/AbsMechanik\n\n");
	}

	// Unknown command
	else printf ("Command unrecognized, enter "
		"\"help\" for a full list of commands\n");

	// All done
	return 0;
}
