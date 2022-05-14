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

	Return 1 if successful, 0 if not.
*/
int initClientList(ClientList* list);

/*
	Reallocate memory to make room for new client and then
	add client info to the dynamic array.

	Return 1 if successfull, 0 if not.
*/
int addToClientList(ClientList* list, struct ConnectionInfo info);

/*
	Check if the sockaddr exists in the list.

	Return 1 if it does, 0 if it doesn't.
*/
int isInClientList(ClientList* list, struct sockaddr_in addr);

//! Old declarations below

int recvMessageFromClient(int socket, Datagram receivedMessage);

int unpackMessage(Datagram packet);

void sendTimeout();

#endif