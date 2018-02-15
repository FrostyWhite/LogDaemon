//
//  main.c
//  LogTester
//
//  Created by Kuura Parkkola on 16/11/17.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "DLogLib.h"

// Global pointers so signal handlers can acces variables //
volatile pid_t *childpid;
volatile int *children;

//					//
//	SIGNAL HANDLERS	//
//					//

void handler_SIGINT(int sig){
	return;
}

void handler_SIGTERM(int sig){
	return;
}

void handler_SIGPIPE(int sig){
	return;
}

//			//
//	TOOLS	//
//			//

// Generate pseudo random integer between p_low and p_high //
int randomgen(int p_low, int p_high, int p_seed){
	srand(p_seed);
	return (rand() % (p_high - p_low)) + p_low;
}

// Connects to the log daemon and sends log entries //
int tester(int childnum){
	
	// Determine number of times the loop runs //
	// and how long program waits before sending
	// connection request
	int loops = randomgen(5, 30, getpid());
	sleep(randomgen(0, 5, loops));
	usleep(randomgen(0, 500000, loops));
	printf("Tester %d starts\n", childnum);
	
	// Some setup //
	log_t log;
	
	char name[11];
	sprintf(name, "Tester%d", childnum);
	strcpy(log.l_name, name);
	
	log.l_fd = DLog_Handshake(name);
	if(log.l_fd == -1){
		printf("ERROR: Failed to create fifo\n");
		exit(-1);
	}else if(log.l_fd == -2){
		printf("ERROR: Failed to open request fifo\n");
		exit(-1);
	}else if(log.l_fd == -3){
		printf("ERROR: Failed to open log fifo\n");
		exit(-1);
	}
	
	char entry[256];
	
	// First write to log and then enter runloop //
	for(int i = 0; i < loops; i++){
		// Send text to the log and wait a random time before sending again //
		sprintf(entry, "Tester %d writing to log for the %d. time out of %d", childnum, i + 1, loops);
		DLog_Write(entry, &log);
		usleep(randomgen(100, 1000000, getpid()));
	}
	
	// Last write and log out //
	printf("Tester %d finishes\n", childnum);
	exit(0);
}

int main(int argc, const char * argv[]) {
	// Some setup and error checking //
	if(argc < 2){
		printf("You have to give a number of testers to spawn\n");
		return 0;
	}
	int arg = atoi(argv[1]);
	if(!arg){
		perror("Invalid argument!");
		return -1;
	}
	int chnum = 0;
	
	pid_t pid;
	
	for(int i = 0; i < arg; i++){
		pid = fork();
		if(pid < 0){
			printf("FAIL");
			printf("%d\n", pid);
		} else if(pid == 0){
			tester(chnum + 1);
		} else {
			chnum++;
		}
	}
	while (wait(NULL) > 0) {}
	return 0;
}
