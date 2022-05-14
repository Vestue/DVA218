#ifndef CLIENT_H
#define CLIENT_H

#include "wen.c"


// Declarations

int sendMessageToServer(int sock, Datagram toSend, struct sockaddr_in destAddr);

int recvMessageFromServer(int socket, Datagram receivedMessage);

void setDefaultHeader(Datagram messageToSend);

#endif