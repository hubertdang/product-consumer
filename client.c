#include "client.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

/*
 * void request()
 *
 * This function sends a request to the server to get a key for a resource.
 *
 * parameters:
 * int msgid			--> the message queue identifier
 * const char* resource --> the constant string of the desired resource
 * bool producer 		--> identifies if the process is a producer or client in output
 *
 * returns:
 * The key to the requested resource.
 */
key_t request(int msgid, const char* resource, bool producer)
{
	struct my_msg_st some_data;		// to store message queue data
	char client_type[10];			// use to identify producer/client in output
	char response[MAX_TEXT];		// to store server response message
	key_t key;						// key to shared resource
	FILE * fd;						// the file descriptor (output file)
	int size;						// the size of the input file

	if (producer)
	{
		sprintf(client_type, "producer");
	}
	else
	{
		sprintf(client_type, "consumer");
	} 

	// prepare request to send to server
	some_data.my_msg_type = CLIENT_MSG_TYPE;
	sprintf(some_data.some_text, "REQUEST;%s", resource);

	// send request to server
	if (msgsnd(msgid, (void *)&some_data, MAX_TEXT, 0) == -1)
	{
    	fprintf(stderr, "[%s]: msgsnd failed\n", client_type);
        exit(EXIT_FAILURE);
    }
	printf("[%s]: sent a request to the server for %s\n", client_type, resource);

	/* can potentially read message meant for another client, so keep reading
	   until confirming that the key is for the resource that you need */

	while(1)
	{
		if (msgrcv(msgid, (void *)&some_data, BUFSIZ, SERVER_MSG_TYPE, 0) == -1)
		{
            fprintf(stderr, "[%s]: msgrcv failed with error: %d\n", client_type, errno);
            exit(EXIT_FAILURE);
        }
		printf("[%s]: read %s from server\n", client_type, some_data.some_text);
		sscanf(some_data.some_text, "RESPONSE;%[^;];%d", response, &key);
		if (!(IS_REQUESTING(response, resource)))
		{
			// send request back into the msg queue as if it was sent by the server
			printf("[%s]: got someone else's key. sending it back\n", client_type);
			if (msgsnd(msgid, (void *)&some_data, MAX_TEXT, 0) == -1)
			{
				fprintf(stderr, "[%s]: msgsnd failed\n", client_type);
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			break;
		}
	}

	// we read the correct message, now extract the key from the server's response
	sscanf(some_data.some_text, "RESPONSE;%[^;];%d", response, &key);
	printf("[%s]: got key %d\n", client_type, key);

	return key;
}
