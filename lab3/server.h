#ifndef SERVER_H
#define SERVER_H

#include "wen.c"

/*Defined values here*/
#define PORT 5555

/*Defined structs and typedefs*/

/*Declared functions here*/

/*
    Allocate memory to a list used for client connection info.
    (malloc)
    Return 1 if successfull, 0 if not.
*/
int initClientList(ClientList* list);


//! Old declarations below

int recvMessageFromClient(int socket, Datagram);

int unpackMessage(Datagram);

int sendTimeout();

#endif