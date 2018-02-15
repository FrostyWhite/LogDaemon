//
//  shouter.c
//  LogDaemon2
//
//  Created by Kuura Parkkola on 09/12/17.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//

#include "shouter.h"

shouterstruct Shouter_Init(){
	shouterstruct shouters;
	shouters.shoutercount = 0;
	FD_ZERO(&shouters.shouters);
	return shouters;
}

shouterstruct *Shouter_Clear(shouterstruct *shouters, pthread_mutex_t *mu_shout){
	pthread_mutex_lock(mu_shout);
	shouters->shoutercount = 0;
	FD_ZERO(&shouters->shouters);
	pthread_mutex_unlock(mu_shout);
	return shouters;
}

shouterstruct *Shouter_Add(shouterstruct *shouters, pthread_mutex_t *mu_shout, int fd_new){
	if(FD_ISSET(fd_new, &shouters->shouters)){
		return NULL;
	}
	pthread_mutex_lock(mu_shout);
	shouters->shoutercount++;
	FD_SET(fd_new, &shouters->shouters);
	pthread_mutex_unlock(mu_shout);
	return shouters;
}

shouterstruct *Shouter_Remove(shouterstruct *shouters, pthread_mutex_t *mu_shout, int fd_rem){
	if(!FD_ISSET(fd_rem, &shouters->shouters)){
		return NULL;
	}
	pthread_mutex_lock(mu_shout);
	shouters->shoutercount--;
	FD_CLR(fd_rem, &shouters->shouters);
	pthread_mutex_unlock(mu_shout);
	return shouters;
}
