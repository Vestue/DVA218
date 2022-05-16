#ifndef SERVER_H
#define SERVER_H

#include "wen.c"

/*Defined values here*/
#define PORT 5555

/*Defined structs and typedefs*/
typedef struct
{
	struct ConnectionInfo *clients;
	size_t size;
} ClientList;

/*Declared functions here*/

/*
	Allocate memory to a list used for client connection info.
	(malloc)

	Return pointer to client list if successful, print error if not.
*/
ClientList initClientList();

/*
    Begin by checking if client is in list.
	Reallocate memory to make room for new client and then
	add client info to the dynamic array.

	Return 1 if successful, 0 if not.
*/
int addToClientList(ClientList *list, struct ConnectionInfo info);

/*
    Attempt to remove client from list.
    First check if client is in list, then remove and reallocate.
    
    Return 1 if successful, 0 if not.
*/
int removeFromClientList(ClientList *list, struct sockaddr_in addr);

/*
	Check if the sockaddr exists in the list.

	Return 1 if it does, 0 if it doesn't.
*/
int isInClientList(ClientList *list, struct sockaddr_in addr);

/*
    Search the list for the client and add the client.
    Return NULL if it can't be found. 
*/
struct ConnectionInfo* findClient(ClientList *list, struct sockaddr_in addr);

/*
	Accepts the connectionrequests
	returns 1 if succesfull
*/
int acceptConnection(int sock, Datagram connRequest, struct sockaddr_in* dest);

//! Temporary declaration for testing
int acceptConnectionInLoop(int sock, Datagram connRequest, struct sockaddr_in* dest);

/*
    Disconnect client and remove from the list.
*/
void closeConnection(ClientList *list, struct sockaddr_in addr);

void interpretPack(int sock, Datagram packet, struct sockaddr_in addr, ClientList *clients);
void interpretWithGBN(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients);
void interpretWithSR(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients);

#endif