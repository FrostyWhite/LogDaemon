//
//  DLogLib.h
//  LogDaemon
//
//  Created by Kuura Parkkola on 16/11/17.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//

#ifndef DLogLib_h
#define DLogLib_h

///////////////////////////////////////////////////////////////
//														//
//						DEPENDENCIES						//
//														//
///////////////////////////////////////////////////////////////

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <errno.h>

///////////////////////////////////////////////////////////////
//***********************************************************//
///////////////////////////////////////////////////////////////
//														//
//					LIBRARY HEADER						//
//														//
///////////////////////////////////////////////////////////////
//***********************************************************//
///////////////////////////////////////////////////////////////

//////////////////////////////////////////////
//					TYPES				//
//////////////////////////////////////////////

// Information about the program calling the functions
typedef struct {
	int l_fd;			// Log FIFO File descriptor
	char l_name[41];	// Program name
} log_t;

//////////////////////////////////////////////
//				Functions				//
//////////////////////////////////////////////

// Set up a link between the daemon and the client
int DLog_Handshake(char *p_name);

// Close the link between the daemon and a linked client
int DLog_Farewell(log_t *p_details);

// Write a given string to the log controlled by the daemon
int DLog_Write(char *p_entry, log_t *p_details);

#endif /* DLogLib_h */
