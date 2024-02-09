# READ ME: PC Problem

## System Requirements:
Linux OS.

## About:
PC Problem is an implementation of the PC problem with System V semaphores.
This program uses two sample files, one is an input file, and the other is
an output file. They are named input_file and output_file. The producer 
reads from input_file to write to a shared buffer, from which the consumer
reads the text, then writes to output_file.

## Note:
This implementation of the PC problem supports only one producer and one
consumer. 

## How To Use:
Compile the program using the compile script in a terminal:

	$ ./compile

This script will produce three executable files: server, producer, consumer.
Run each program in that order on separate terminals:

	$ ./server

	$ ./producer

	$ ./consumer
