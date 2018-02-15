//
//  shouter.h
//  LogDaemon2
//
//  Created by Kuura Parkkola on 9.12.2017.
//  Copyright © 2017 Kuura Parkkola. All rights reserved.
//

#ifndef shouter_h
#define shouter_h

#include <sys/select.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct{
	int shoutercount;
	fd_set shouters;
} shouterstruct;

shouterstruct Shouter_Init();

shouterstruct *Shouter_Clear(shouterstruct *shouters, pthread_mutex_t *mu_shout);

shouterstruct *Shouter_Add(shouterstruct *shouters, pthread_mutex_t *mu_shout, int fd_new);

shouterstruct *Shouter_Remove(shouterstruct *shouters, pthread_mutex_t *mu_shout, int fd_rem);

#endif /* shouter_h */
