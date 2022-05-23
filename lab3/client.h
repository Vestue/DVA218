#ifndef CLIENT_H
#define CLIENT_H

/*Defined values here*/

/* Declarations here */
void printCursorThingy(void);



void interpretPack_sender(ConnectionInfo *server, int currentSeq);
void interpretPack_sender_GBN(Datagram receivedDatagram, ConnectionInfo *server);
void interpretPack_sender_SR(Datagram receivedDatagram, ConnectionInfo *server, int currentSequence);

void checkTimedOutPacks(ConnectionInfo *server, int *currentSeq);
void checkTimeout_GBN(ConnectionInfo *server, int *currentSeq);
void checkTimeout_SR(ConnectionInfo *server, int *currentSeq);
void selectTimeout(int signal);

#endif