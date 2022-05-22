#ifndef CLIENT_H
#define CLIENT_H

#include "wen.c"

/*Defined values here*/

/* Declarations here */
int writeMessage(ConnectionInfo *server, char* message, int* currentSeq);
int writeMessageGBN(ConnectionInfo *server, char* message, int* currentSeq);
int writeMessageSR(ConnectionInfo *server, char* message, int* currentSeq);

#endif