#ifndef SERVER_H
#define SERVER_H

#include "wen.c"

/*Defined values here*/

/*Defined structs and typedefs*/

/*Declared functions here*/


/*
    Disconnect client and remove from the list.
*/
void closeConnection(ClientList *list, struct sockaddr_in addr);

#endif