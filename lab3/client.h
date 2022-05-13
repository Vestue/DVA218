#ifndef CLIENT_H
#define CLIENT_H

#include "wen.c"

/*Defined values*/
#define SYN 1;
#define ACK 2;
#define FIN 4;
// Declarations

int sendMessageToServer(int socket);

int recvMessageFromServer(int socket, Datagram receivedMessage);

void setDefaultHeader(Datagram messageToSend)

#endif