#ifndef SERVER_H
#define SERVER_H

#include "wen.h"

/*Defined values here*/
#define PORT 5555

/*Defined structs and typedefs*/
struct ClientInfo
{
    struct sockaddr_in addr;
    int expectedSeqNum;
    int FIN_SET;
};

/*Declared functions here*/

/* Creates a new socket, binds it and returns the socket */
int createSocket(void);

int recvMessageFromClient(int socket, Datagram);

int unpackMessage(Datagram);

void setDefaultHeader(Datagram messageToSend)

int sendTimeout();



#endif