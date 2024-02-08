#ifndef SEM_UTL_H
#define SEM_UTL_H
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include "semun.h"

#define MUTEX_COUNT 1	// set count to 1 for mutual exclusion
#define SYNC_COUNT	0	// set count to 0 for synchronization		
#define K_COUNT		10	// this implementation has k = 10 buffers

int set_semvalue(int sem_id, int count);
void del_semvalue(int sem_id);
int semaphore_p(int sem_id);
int semaphore_v(int sem_id);
#endif
