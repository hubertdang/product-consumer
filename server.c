#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/msg.h>
#include <time.h>
#include "server.h"

int main()
{
	int running = 1;				// to keep the main loop running
	int msgid;						// the idenfitier for the message queue
	struct my_msg_st some_data;		// the data to send/receive
	key_t semS_key;					// the key for semaphore S
	key_t semE_key;					// the key for semaphore E
	key_t semN_key;					// the key for semaphore N
	key_t buffer_key;				// the key for the shared buffer
	char request[MAX_TEXT];			// for storing the client's requested resource identifier

	// set up message queue first
	msgid = msgget((key_t)MSG_QUEUE, 0666 | IPC_CREAT);
    if (msgid == -1)
	{
        fprintf(stderr, "[server]: msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
	printf("[server]: finished setting up the message queue\n");

	// generate random keys for shared resources
	srand(time(NULL));
	semS_key = rand() % 9000 + 1000;
	printf("[server]: semaphore S key is %d\n", semS_key);
	semE_key = rand() % 9000 + 1000;
	printf("[server]: semaphore E key is %d\n", semE_key);
	semN_key = rand() % 9000 + 1000;
	printf("[server]: semaphore N key is %d\n", semN_key);
	buffer_key = rand() % 9000 + 1000;
	printf("[server]: shared buffer key is %d\n", buffer_key);

	while(running)
	{
		// read request from client
		if (msgrcv(msgid, (void *)&some_data, BUFSIZ, CLIENT_MSG_TYPE, 0) == -1) 
		{
			fprintf(stderr, "[server]: msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		printf("[server]: %s was read\n", some_data.some_text);

		// process client request
		sscanf(some_data.some_text, "REQUEST;%s", request);

		if (IS_REQUESTING(request, END))	// a client requested to end the server
		{
			running = 0;
		}
		else if (IS_REQUESTING(request, SEM_S))
		{
			printf("[server]: sending semaphore S key\n");	
			sprintf(some_data.some_text, "RESPONSE;semS;%d", semS_key);
		}
		else if (IS_REQUESTING(request, SEM_E))
		{
			printf("[server]: sending semaphore E key\n");	
			sprintf(some_data.some_text, "RESPONSE;semE;%d", semE_key);
		}
		else if (IS_REQUESTING(request, SEM_N))
		{
			printf("[server]: sending semaphore N key\n");	
			sprintf(some_data.some_text, "RESPONSE;semN;%d", semN_key);
		}
		else if (IS_REQUESTING(request, BUFFER))
		{
			printf("[server]: sending shared buffer key\n");
			sprintf(some_data.some_text, "RESPONSE;buffer;%d", buffer_key);
		}

		some_data.my_msg_type = SERVER_MSG_TYPE;	// change type so client reads response

		// send response to client
		if (msgsnd(msgid, (void *)&some_data, MAX_TEXT, 0) == -1) 
		{
			fprintf(stderr, "[server]: msgsnd failed\n");
			exit(EXIT_FAILURE);
		}
	}
	if (msgctl(msgid, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "[server]: msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}
}
