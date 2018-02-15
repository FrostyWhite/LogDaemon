//
//  DLogLib.c
//  LogTester
//
//  Created by Kuura Parkkola on 09/12/17.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//

///////////////////////////////////////////////////////////////
//***********************************************************//
///////////////////////////////////////////////////////////////
//														//
//				LIBRARY IMPLEMENTATION					//
//														//
///////////////////////////////////////////////////////////////
//***********************************************************//
///////////////////////////////////////////////////////////////

#include "DLogLib.h"

char *Internal_CreateEntry(char *p_entry, char *p_key);

int DLog_Handshake(char *p_name){
	char path[256]; int fd_req, fd_log;
	sprintf(path, "./links/%d%s", getpid(), p_name);
	int mk = mkfifo(path, 0777);
	if(mk < 0){
		return -1;
	}
	fd_req = open("./access", O_WRONLY);
	if(fd_req < 0){
		return -2;
	}
	write(fd_req, path, strlen(path));
	close(fd_req);
	fd_log = open(path, O_WRONLY);
	if(fd_log < 0){
		return -3;
	}
	return fd_log;
}

int DLog_Write(char *p_entry, log_t *p_details){
	
	if(strlen(p_entry) + strlen(p_details->l_name) > 219) return -1;
	
	// Generate message //
	char msg[256] = { 0 };
	sprintf(msg, "%d: %s %s", getpid(), p_details->l_name, p_entry);
	
	// Write and return //
	int n = (int) write(p_details->l_fd, msg, strlen(msg) + 1);
	return n;
}
