#include <sys/msg.h>
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sem_utl.h"
#include <sys/shm.h>
#include "string.h"

#define FILE_NAME "output_file"


int main()
{
	int size;						// the size of the input file
	int msgid;						// the message queue identifier
	struct size_msg_st size_msg;	// for sending file size to consumer
	int semS_id, semE_id, semN_id;	// semaphore IDs for semaphore S, E, and N
	int sequence_number = 0;		// sequence number to check with
	struct buffer_st * buffer;		// pointer to the shared buffer
	int out = 0;					// pointer to know where to consume 
	FILE * fd;						// output file descriptor
	char w[1024];					// the data consumed from buffer
	int buffer_id;					// identifier for the shared buffer
	int running = 1;				// controls main consumer loop
	struct my_msg_st end_msg;		// for sending message to tell server to end in cleanup

	sleep(1);	// wait a bit for the producer to create semaphores and get file size

	// open output file for writing
	fd = fopen(FILE_NAME, "w");		
	if (fd == NULL)
	{
		fprintf(stderr, "[consumer]: error opening output file\n");
		exit(EXIT_FAILURE);
	}
	printf("[consumer]: opened the output file for writing\n");

	// set up message queue first to get keys from server and read file size from producer 
	msgid = msgget((key_t)MSG_QUEUE, 0666 | IPC_CREAT);
    if (msgid == -1)
	{
        fprintf(stderr, "[consumer]: msgget failed with error: %d\n", errno);
		fclose(fd);	// Close the file before exiting
        exit(EXIT_FAILURE);
    }
	printf("[consumer]: finished setting up the message queue\n");

	// obtain the semaphore IDs of the semaphores created by the producer
	semS_id = semget(request(msgid, SEM_S, CONSUMER), 1, 0666 | IPC_CREAT);
	semE_id = semget(request(msgid, SEM_E, CONSUMER), 1, 0666 | IPC_CREAT);
	semN_id = semget(request(msgid, SEM_N, CONSUMER), 1, 0666 | IPC_CREAT);
	printf("[consumer]: obtained the semaphore IDs\n");

	// read message from producer telling what the input file size is
	if (msgrcv(msgid, (void *)&size_msg, sizeof(int), SIZE_MSG_TYPE, 0) == -1)
	{
		fprintf(stderr, "[consumer]: msgrcv failed with error: %d\n", errno);
		fclose(fd);	// Close the file before exiting
		exit(EXIT_FAILURE);
	}
	size = size_msg.file_size;
	printf("[consumer]: producer says file size is %d bytes\n", size_msg.file_size);

	// get identifier for the shared buffer and assign it to this process
	buffer_id = shmget(request(msgid, BUFFER, CONSUMER), sizeof(struct buffer_st) * K_COUNT, 0666 | IPC_CREAT);
	if (buffer_id == -1)
	{
		fprintf(stderr, "[producer]: shmget failed\n");
		fclose(fd);	// Close the file before exiting
		exit(EXIT_FAILURE);
	}
	buffer = shmat(buffer_id, (void *)0, 0);
	if (buffer == (void *)-1)
	{
		fprintf(stderr, "[consumer]: shmat failed\n");
		fclose(fd);	// Close the file before exiting
		exit(EXIT_FAILURE);
	}
	printf("[consumer]: memory for shared buffer attached\n");
	buffer = (struct buffer_st *)buffer;

	while(running)
	{
		printf("[consumer]: wait(N)\n");	
		semaphore_p(semN_id);	// wait(N)
		
		printf("[consumer]: wait(S)\n");
		semaphore_p(semS_id);	// wait(S)	

		// critical section
		if (buffer[out].sequence_number != sequence_number)
		{
			printf("[consumer]: error! sequence number does not match! expected: %d, actual: %d\n", sequence_number, buffer[out].sequence_number);
			fclose(fd);	// Close the file before exiting
			exit(EXIT_FAILURE);
		}
		strcpy(w, buffer[out].text);	// consume
		out = (out + 1) % K_COUNT;
		// critical section
		
		printf("[consumer]: signal(S)\n");
		semaphore_v(semS_id);	// signal(S)

		printf("[consumer]: signal(E)\n");
		semaphore_v(semE_id);	// signal(E)

		sequence_number += strlen(w);	// update sequence number with the number of bytes read
		if (sequence_number == size) 
		{
			running = 0;
		}

		// write to output file
		if (fputs(w, fd) == EOF) 
		{
			fprintf(stderr, "[consumer]: error writing to the file\n");
			fclose(fd);	// Close the file before exiting
			exit(EXIT_FAILURE);
		}
		printf("[consumer]: wrote to output file\n");
	}

	printf("[consumer]: success! %d total bytes read\n", sequence_number);

	// cleanup
	
	// prepare request to end server process
	end_msg.my_msg_type = CLIENT_MSG_TYPE;
	sprintf(end_msg.some_text, "REQUEST;%s", END);

	// send request to server
	if (msgsnd(msgid, (void *)&end_msg, MAX_TEXT, 0) == -1)
	{
    	fprintf(stderr, "[consumer]: msgsnd failed\n");
        exit(EXIT_FAILURE);
    }
	printf("[consumer]: sent a request to the server to end\n");

	fclose(fd);	// Close the file before exiting

	if (shmdt(buffer) == -1)	// detach shared buffer
	{
		fprintf(stderr, "[producer]: shmdt failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
