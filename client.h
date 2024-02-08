#ifndef CLIENT_H
#define CLIENT_H
#include <stdbool.h>
#include "server.h"
#include <sys/msg.h>

#define PRODUCER 1	// for customizing output in request()
#define CONSUMER 0	// for customizing output in request()

struct size_msg_st			// message structure for producer to send file size to consumer
{
	long int my_msg_type;
	int file_size;
};

struct buffer_st		// buffer used by producers and consumers
{
	char text[1024];
	int sequence_number;	// offset from beginning of document
	int count;
};

key_t request(int msgid, const char* resource, bool producer);
#endif
