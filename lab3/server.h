#ifndef SERVER_H
#define SERVER_H

#include "wen.c"

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

int recvMessageFromClient(int socket, Datagram);

int unpackMessage(Datagram);

int sendTimeout();



#endif