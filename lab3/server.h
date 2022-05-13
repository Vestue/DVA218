#ifndef SERVER_H
#define SERVER_H

#include "wen.h"

/*Defined values here*/
#define PORT 5555
#define MAXLENGTH 1024
#define WINDOWSIZE 64
#define MAXSEQNUM 128
#define SYN 1;
#define ACK 2;
#define FIN 4;

/*Declared functions here*/

/* Creates a new socket, binds it and returns the socket */
int createSocket(void);

int sendMessageToClient(int socket);

int recvMessageFromClient(int socket, Datagram receivedMessage);

int unpackMessage(Datagram messageToUnpack);

#endif