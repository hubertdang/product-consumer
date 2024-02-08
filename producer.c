#include <sys/msg.h>
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sem_utl.h"
#include <sys/shm.h>
#include "string.h"

#define FILE_NAME "input_file"


int main()
{
	FILE * fd;						// the file descriptor
	int size;						// the size of the file
	int msgid;						// the message queue identifier
	struct size_msg_st size_msg;	// for sending file size to consumer
	int semS_id, semE_id, semN_id;	// semaphore IDs for semaphore S, E, and N
	int buffer_id;					// shared buffer ID
	struct buffer_st *buffer;		// pointer to the shared buffer
	char v[1024];					// the thing produced
	int running = 1;				// to control producer loop 
	int in = 0;						// in pointer to know where to append the buffer
	int sequence_number = 0;		// the sequence number to update when we read

	// open the file for reading
	fd = fopen(FILE_NAME, "r");		
	printf("[producer]: opened the input file for reading\n");

	// get file size
	fseek(fd, 0, SEEK_END);				// seek to end of file
	size_msg.file_size = ftell(fd);		// get current file pointer
	fseek(fd, 0, SEEK_SET);				// seek back to beginning of file
	printf("[producer]: read file size, %d bytes\n", size_msg.file_size);

	// set up message queue first to get keys from server and send file size to consumer 
	msgid = msgget((key_t)MSG_QUEUE, 0666 | IPC_CREAT);
    if (msgid == -1)
	{
        fprintf(stderr, "[producer]: msgget failed with error: %d\n", errno);
		fclose(fd);	// Close the file before exiting
        exit(EXIT_FAILURE);
    }
	printf("[producer]: finished setting up the message queue\n");

	// initialize and set the semaphores
	semS_id = semget(request(msgid, SEM_S, PRODUCER), 1, 0666 | IPC_CREAT);
	semE_id = semget(request(msgid, SEM_E, PRODUCER), 1, 0666 | IPC_CREAT);
	semN_id = semget(request(msgid, SEM_N, PRODUCER), 1, 0666 | IPC_CREAT);
	set_semvalue(semS_id, MUTEX_COUNT);
	set_semvalue(semE_id, K_COUNT);
	set_semvalue(semN_id, SYNC_COUNT);
	printf("[producer]: initialized and set semaphores\n");

	// create shared buffer and attach it to this process' address space
	buffer_id = shmget(request(msgid, BUFFER, PRODUCER), sizeof(struct buffer_st) * K_COUNT, 0666 | IPC_CREAT);
	if (buffer_id == -1)
	{
		fprintf(stderr, "[producer]: shmget failed\n");
		fclose(fd);	// Close the file before exiting
		exit(EXIT_FAILURE);
	}
	buffer = shmat(buffer_id, (void *)0, 0);
	if (buffer == (void *)-1)
	{
		fprintf(stderr, "[producer]: shmat failed\n");
		fclose(fd);	// Close the file before exiting
		exit(EXIT_FAILURE);
	}
	printf("[producer]: memory for shared buffer attached\n");
	buffer = (struct buffer_st *)buffer;

	// send file size to consumer
	size_msg.my_msg_type = SIZE_MSG_TYPE;
	if (msgsnd(msgid, (void *)&size_msg, sizeof(int), 0) == -1)
	{
    	fprintf(stderr, "[producer]: msgsnd failed\n");
		fclose(fd);	// Close the file before exiting
        exit(EXIT_FAILURE);
    }
	printf("[producer]: sent the file size to the consumer\n");

	// producer loop
	while(running)
	{
		// produce v by reading from input file and storing the bytes
		if (fgets(v, sizeof(v), fd) == NULL)
		{
			if (feof(fd))
			{
				fprintf(stderr, "[producer]: end of file reached\n");
				fclose(fd);	// Close the file before exiting
				running = 0;	// no more to read, stop loop
			}
			else if (ferror(fd))
			{
				fprintf(stderr, "[producer]: error reading input file\n");
				fclose(fd);	// Close the file before exiting
				exit(EXIT_FAILURE);
			}
		}
		printf("[producer]: produced v\n");

		printf("[producer]: wait(E)\n");	
		semaphore_p(semE_id);	// wait(E)

		printf("[producer]: wait(S)\n");
		semaphore_p(semS_id);	// wait(S)

		// critical section
		strcpy(buffer[in].text, v);
		buffer[in].sequence_number = sequence_number;
		buffer[in].count = strlen(v);
		in = (in + 1) % K_COUNT;
		// critical section

		printf("[producer]: signal(S)\n");
		semaphore_v(semS_id);	// signal(S)
		
		printf("[producer]: wrote v to buffer\n");
		sequence_number += strlen(v);	

		printf("[producer]: signal(N)\n");
		semaphore_v(semN_id);	// signal(N)
	}

	sleep(1);	// let consumer finish
	
	// clean up
	del_semvalue(semS_id);
	del_semvalue(semE_id);
	del_semvalue(semN_id);
	
	if (shmdt(buffer) == -1)	// detach shared buffer
	{
		fprintf(stderr, "[producer]: shmdt failed\n");
		exit(EXIT_FAILURE);
	}	

	if (shmctl(buffer_id, IPC_RMID, 0) == -1)	// delete shared buffer
	{
		fprintf(stderr, "[producer]: shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);	
}
