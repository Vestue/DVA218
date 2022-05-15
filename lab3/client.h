#ifndef CLIENT_H
#define CLIENT_H

#include "wen.c"

/*Defined values here*/

// Set port to 0 so OS assigns any avaible port
#define PORT 0
#define SERVERPORT 5555

// Declarations

int sendMessageToServer(int sock, Datagram toSend, struct sockaddr_in destAddr);

int recvMessageFromServer(int socket, Datagram receivedMessage);

#endif