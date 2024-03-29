#ifndef CLIENT_H
#define CLIENT_H

#include "wen.c"

/* Declarations here */
void printCursorThingy(void);

int writeMessage(ConnectionInfo *server, char* message, int *currentSeq);
int writeMessageGBN(ConnectionInfo *server, char* message, int currentSeq);
int writeMessageSR(ConnectionInfo *server, char* message, int *currentSeq);

void interpretPack_sender(ConnectionInfo *server, int currentSeq);
void interpretPack_sender_GBN(Datagram receivedDatagram, ConnectionInfo *server);
void interpretPack_sender_SR(Datagram receivedDatagram, ConnectionInfo *server, int currentSequence);

void checkTimedOutPacks(ConnectionInfo *server, int *currentSeq);
void checkTimeout_GBN(ConnectionInfo *server, int *currentSeq);
void checkTimeout_SR(ConnectionInfo *server, int *currentSeq);
void selectTimeout(int signal);

#endif