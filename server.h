#ifndef SERVER_H
#define SERVER_H

// message queue stuff
#define MSG_QUEUE 1235
#define CLIENT_MSG_TYPE 1	// client messages identified by message type 1
#define SERVER_MSG_TYPE 2	// server messages identified by message type 2
#define SIZE_MSG_TYPE	3	// for the size message sent by producer to consumer
#define MAX_TEXT 512

struct my_msg_st 			// the struct to be sent/received in the message queue
{
	long int my_msg_type;
	char some_text[MAX_TEXT];
};

#define END "END"	// used to terminate server

// resource request message identifiers
#define SEM_S "semS"
#define SEM_E "semE"
#define SEM_N "semN"
#define BUFFER "buffer"

#define IS_REQUESTING(request, resource) (strcmp(request, resource) == 0)
#endif
