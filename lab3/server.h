#ifndef SERVER_H
#define SERVER_H

#include "wen.c"

/*Defined values here*/

/*Defined structs and typedefs*/

/*Declared functions here*/



/*
	Accepts the connectionrequests
	returns 1 if succesfull
    
    ! Remove when start to use main loop in server
*/
int TEMPacceptConnection(int sock, Datagram connRequest, struct sockaddr_in* dest);

/*
    Disconnect client and remove from the list.
*/
void closeConnection(ClientList *list, struct sockaddr_in addr);

#endif