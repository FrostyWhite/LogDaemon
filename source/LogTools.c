//
//  LogTools.c
//  LogDaemon2
//
//  Created by Kuura Parkkola on 8.12.2017.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//

#include "LogTools.h"

// Creates a new log file name based on current time
int Internal_GenerateLOGMod(char *p_target);

// Adds a timestamp to the string given in p_target
char *Internal_GenerateTimestamp(char *p_target, char *p_source, int p_pid);

int LogTools_CreateLog(char *p_dir){
	
	// Defining resources //
	char dir[256]; char mod[20];
	
	// Generating filename based on current time //
	Internal_GenerateLOGMod(mod);		//ERRORS
	sprintf(dir, "%s/logs/%s", p_dir, mod);
	
	// Opening file for writing //
	int fd = open(dir, O_WRONLY|O_APPEND|O_CREAT, 0444);	//ERRORS
	return fd;
}

int LogTools_WriteToLog(char *p_entry, int p_fd, pthread_mutex_t *p_lock, int p_pid){
	
	// Lock the file write to it and release the lock //
	char tbw[256];
	Internal_GenerateTimestamp(tbw, p_entry, p_pid);
	if(strlen(tbw) > 256) return -1;
	
	pthread_mutex_lock(p_lock);
	size_t n = write(p_fd, tbw, strlen(tbw));
	write(p_fd, "\n", 2);
	pthread_mutex_unlock(p_lock);
	return (int) n;
}

int Internal_GenerateLOGMod(char *p_target){
	// Getting the time and turning it into a filename
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(p_target, "%04d%02d%02d:%02d%02d%02d.txt", 1900 + tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	return 1;
}

char *Internal_GenerateTimestamp(char *p_target, char *p_source, int p_pid){
	// Get current date and time, second precision //
	const char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	// Extend to microsecond precision //
	struct timeval u_sec;
	gettimeofday(&u_sec, NULL);
	
	char pid[7];
	if(p_pid == 0){
		strncpy(pid, "", 1);
	} else {
		sprintf(pid, "%d", p_pid);
	}
	
	// Generate message //
	sprintf(p_target, "%s %d %02d:%02d:%02d.%03d %s %s", months[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (int) u_sec.tv_usec / 1000, pid, p_source);
	return p_target;
}
