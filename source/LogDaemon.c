//
//  main.c
//  LogDaemon2
//
//  Created by Kuura Parkkola on 8.12.2017.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//



////////////////////////////////////////////
	//	***  LIBRARIES AND MACROS  ***	//
////////////////////////////////////////////

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/poll.h>
#include <errno.h>
#include <signal.h>

#include "LogTools.h"
#include "shouter.h"

#define IDLE_TIMEOUT 15000 // Milliseconds



////////////////////////////////////////////
	//	****  TYPE DEFINITIONS	****	//
////////////////////////////////////////////

// Stores some local variables for simplicity
typedef struct{
	char origin[256];
	int log_fd;
	int fifo_fd;
	char *fifopath;
	char dir_links[256];
	char dir_logs[256];
} valset;

typedef struct{
	pthread_mutex_t *mu_log;
	char flags;
	int *logfd;
	shouterstruct *shouters;
	int *fifofd;
	char *thread_input;
	char *master_input;
} sigd_t;



////////////////////////////////////////////
	//	********  GLOBALS	********	//
////////////////////////////////////////////

sigd_t *signaldata;



////////////////////////////////////////////
	//	****  SIGNAL HANDLING	****	//
////////////////////////////////////////////

void termhandler(int sig){
	if(signaldata->flags & 1){
		LogTools_WriteToLog("Log Daemon:Master Program ends with a signal", *signaldata->logfd, signaldata->mu_log, getpid());
		close(*signaldata->logfd);
	} if(signaldata->flags & 2){
		signaldata->shouters->shoutercount = 0;
		FD_ZERO(&signaldata->shouters->shouters);
	} if(signaldata->flags & 4){
		close(*signaldata->fifofd);
	} if(signaldata->flags & 8){
		free(signaldata->master_input);
	} if(signaldata->flags & 16){
		free(signaldata->thread_input);
	}
	exit(1);
}



////////////////////////////////////////////
	//	****	CLIENT THREAD	****	//
////////////////////////////////////////////

void *client_thread(void *arg){
	arg_t *args = (arg_t *) arg; int status = 0; 
	while(*args->run == 0){
		if(*args->waited){
			pthread_exit(NULL);
		}
		usleep(100000);
	}
	char *input = calloc(256, sizeof(char));
	signaldata->thread_input = input;
	signaldata->flags |= 16;
	
	int shtcsafe; int idle_laps = 0;
	struct timeval timeo; fd_set copy;
	while (*args->run || *args->active){
		timeo.tv_sec = 0;
		timeo.tv_usec = 5000;
		FD_ZERO(&copy);
		copy = args->shouters->shouters;
		shtcsafe = args->shouters->shoutercount;
		pthread_mutex_lock(args->mu_shout);
		status = select(256, &copy, NULL, NULL, &timeo);
		if (status > 0) {
			*args->active = 1;
			for(int i = 0; i < 256; i++){
				if (FD_ISSET(i, &copy)){
					
					read(i, input, 255);
					LogTools_WriteToLog(input, args->log_fd, args->mu_log, 0);
				}
			}
		} else if (status < 0) {
			LogTools_WriteToLog("LogDaemon:ClientThread Execution interrupted by a signal", args->log_fd, args->mu_log, getpid());
		} else {
			idle_laps++;
		}
		pthread_mutex_unlock(args->mu_shout);
		if(idle_laps > 3000){
			*args->active = 0;
		}
		if(*args->waited){
			break;
		}
	}
	Shouter_Clear(args->shouters, args->mu_shout);
	LogTools_WriteToLog("LogDaemon:ClientThread Exiting", args->log_fd, args->mu_log, getpid());
	pthread_exit(NULL);
}



////////////////////////////////////////////
	//	********	MAIN	********	//
	//	*** INIT + MASTER THREAD ***	//
////////////////////////////////////////////

int main(int argc, const char * argv[]) {
	
////////////////////////
	// signal handling
////////////////////////
	sigd_t sigdata;
	sigdata.flags = 0;
	sigdata.logfd = NULL;
	sigdata.shouters = NULL;
	sigdata.fifofd = NULL;
	sigdata.master_input = NULL;
	sigdata.thread_input = NULL;
	
	signaldata = &sigdata;
	
	sigset_t terminators, block;
	sigemptyset(&terminators);
	sigaddset(&terminators, SIGINT);
	sigaddset(&terminators, SIGTERM);
	sigaddset(&terminators, SIGSEGV);
	sigaddset(&terminators, SIGQUIT);
	sigemptyset(&block);
	sigaddset(&block, SIGUSR1);
	sigaddset(&block, SIGUSR2);
	sigaddset(&block, SIGHUP);
	sigaddset(&block, SIGALRM);
	sigprocmask(SIG_SETMASK, &block, NULL);
	
	struct sigaction s_close;
	s_close.sa_mask = terminators;
	s_close.sa_handler = termhandler;
	s_close.sa_flags = 0;
	
	
////////////////////////
	// daemon creation
////////////////////////
	
	// Define resources //
	pid_t pid, sid;
	valset vals;
	pthread_mutex_t mu_shout;
	pthread_mutex_t mu_log;
	
	// Get current run directory //
	if (!getcwd(vals.origin, sizeof(vals.origin) - 26)){
		perror("Program was unable do determine the running directory, creating the daemon failed.");
		exit(-2);
	}
	
	// Create new process to create daemon into //
	pid = fork();
	if(pid > 0){
		return 0;
	} else if(pid < 0){
		perror("Creation of a child process failed, creating the daemon failed.");
		exit(-3);
	}
	
	
///////////////////////
	// initialization
///////////////////////
	
	umask(022);
	
	// Creating directory structure //
	strcpy(vals.dir_logs, vals.origin);
	strcpy(vals.dir_links, vals.origin);
	strcat(vals.dir_logs, "/logs");
	strcat(vals.dir_links, "/links");
	mkdir(vals.dir_logs, 0777);
	mkdir(vals.dir_links, 0777);
	
	// Create a new log file //
	vals.log_fd = LogTools_CreateLog(vals.origin);
	if(vals.log_fd < 0){
		perror("Opening log file failed, creating the daemon failed.");
		exit(-4);
	}
	
	signaldata->logfd = &vals.log_fd;
	signaldata->flags |= 1;
	
	// Create a mutex lock to eliminate //
	// race conditions when threads write
	// to the log
	
	if(pthread_mutex_init(&mu_log, NULL) != 0){
		perror("Setting log mutex failed, creating the daemon failed.");
		exit(-8);
	}
	
	signaldata->mu_log = &mu_log;
	
	if(pthread_mutex_init(&mu_shout, NULL) != 0){
		perror("LogDaemon: INIT: Client list mutex init failed.");
		exit(-8);
	}
	
	LogTools_WriteToLog("LogDaemon: INIT: Log setup done", vals.log_fd, &mu_log, getpid());
	
	// Give process a new session id //
	sid = setsid();
	if(sid < 0){
		LogTools_WriteToLog("Setting new session id failed, exiting", vals.log_fd, &mu_log, getpid());
		exit(-5);
	}
	
	// Close standard filestreams for security reasons //
	if(close(STDIN_FILENO) + close(STDOUT_FILENO) + close(STDERR_FILENO) != 0){
		LogTools_WriteToLog("Closing standard I/O failed, exiting", vals.log_fd, &mu_log, getpid());
		exit(-7);
	}
	
	/**** ACCESS ONLY VIA SHOUTER LIBRARY! ****/
	/* OR SIGNAL HANDLERS IF REALLY NECESSARY */
	////////////////////////////////////////////
	shouterstruct s_shouters = Shouter_Init();//
	////////////////////////////////////////////
	signaldata->shouters = &s_shouters;
	signaldata->flags |= 2;
	
	pthread_t tid;
	int thread_run = 0;
	int thread_active = 0; // Only accessed by sighandlers and the client thread
	int thread_waited = 0;
	
	arg_t toThread;
	toThread.run = &thread_run;
	toThread.active = &thread_active;
	toThread.waited = &thread_waited;
	toThread.origin = vals.origin;
	toThread.log_fd = vals.log_fd;
	toThread.shouters = &s_shouters;
	toThread.mu_shout = &mu_shout;
	toThread.mu_log = &mu_log;
	
	pthread_create(&tid, NULL, client_thread, &toThread);
	
	struct pollfd pfd; int status;
	pfd.fd = vals.fifo_fd;
	pfd.events = (POLLIN);
	
	mkfifo("./access", 0777);
	vals.fifo_fd = open("./access", O_RDONLY | O_NONBLOCK);
	signaldata->fifofd = &vals.fifo_fd;
	signaldata->flags |= 4;
	
	
///////////////////////////
	// master thread loop
///////////////////////////
	
	// Daemon runloop //
	char *input = calloc(256, sizeof(char));
	signaldata->master_input = input;
	signaldata->flags |= 8;
	
	int new_fd;
	while(1){
		status = poll(&pfd, 1, IDLE_TIMEOUT);
		if(status > 0){
			read(vals.fifo_fd, input, 256);
			new_fd = open(input, O_RDONLY);
			if(new_fd >= 0){
				Shouter_Add(&s_shouters, &mu_shout, new_fd);
				*toThread.run = 1;
				LogTools_WriteToLog("LogDaemon:MasterThread New client entered", vals.log_fd, &mu_log, getpid());
			}
		} else if (status == 0 && thread_active == 0){
			if (thread_active == 0) {
				*toThread.run = 0;
				close(vals.fifo_fd);
				LogTools_WriteToLog("LogDaemon:MasterThread Both threads have gone idle, exiting", vals.log_fd, &mu_log, getpid());
				break;
			} else {
				LogTools_WriteToLog("LogDaemon:MasterThread No new clients during last IDLE period", vals.log_fd, &mu_log, getpid());
			}
		} else {
			if(errno == EINTR){
				LogTools_WriteToLog("LogDaemon:MasterThread Execution interrupted by a signal", vals.log_fd, &mu_log, getpid());
			}
		}
	}
	*toThread.waited = 1;
	pthread_join(tid, NULL);
	close(vals.log_fd);
	return 0;
}
