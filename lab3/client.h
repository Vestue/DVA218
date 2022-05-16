#ifndef CLIENT_H
#define CLIENT_H

#include "wen.c"

/*Defined values here*/

// Set port to 0 so OS assigns any avaible port
#define PORT 0
#define SERVERPORT 5555

/* Declarations here */

/*
    Setup information required for connection, go through the connection handshake
    with the server.
    Return sequence number sent in the server SYN+ACK.
*/
int setupConnection(int* sock, char* hostName, struct sockaddr_in* destAddr);

#endif