#include "sem_utl.h"

/*
 * int set_semvalue()
 *
 * Initializes the semaphore using the SETVAL command in a semctl 
 * call. We need to do this before we can use the semaphore.
 *
 * parameters:
 * int sem_id	--> semaphore identifier
 * int count	--> semaphore count	
 */ 
int set_semvalue(int sem_id, int count)
{
    union semun sem_union;

    sem_union.val = count;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}


/*
 * int del_semvalue()
 * 
 * Uses the command IPC_RMID to remove the semaphore's ID.
 *
 * parameters:
 * int sem_id	--> semaphore identifier
 */
void del_semvalue(int sem_id)
{
    union semun sem_union;
    
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}


/*
 * int semaphore_p()
 *
 * Equivalent to semaphore wait() operation.
 *
 * parameters:
 * int sem_id	--> semaphore identifier
 */
int semaphore_p(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }

    return(1);
}


/*
 * int semaphore_v()
 *
 * Equivalent to semaphore signal() operation.
 *
 * parameters:
 * int sem_id	--> semaphore identifier
 */
int semaphore_v(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }

    return(1);
}


