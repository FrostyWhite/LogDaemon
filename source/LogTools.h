//
//  LogTools.h
//  LogDaemon2
//
//  Created by Kuura Parkkola on 8.12.2017.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//

#ifndef LogTools_h
#define LogTools_h

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <poll.h>
#include <errno.h>

#include "shouter.h"

// Required arguments for a newly generated thread

typedef struct{
	int *run;
	int *active;
	int *waited;
	char *origin;
	int log_fd;
	shouterstruct *shouters;
	pthread_mutex_t *mu_shout;
	pthread_mutex_t *mu_log;
} arg_t;

// Client information in a compact package
typedef struct{
	pid_t pid;		// Client process id
	char pname[200];	// Client process name
	char key[7];		// Client authorisation key
	char path[256];	// Client fifo filepath
	int file_desc;	// Client fifo file descriptor
} client_t;

//////////////////////////
// Interface functions //
////////////////////////

//
// LOG MANAGEMENT TOOLS
//

// Creates a new log file into the 'logs' folder
int LogTools_CreateLog(char *p_dir);

// Writes the given message into the log file behind p_fd
// Race conditions are evaded via mutex 'p_lock'
int LogTools_WriteToLog(char *p_entry, int p_fd, pthread_mutex_t *p_lock, int p_pid);



#endif /* LogTools_h */
