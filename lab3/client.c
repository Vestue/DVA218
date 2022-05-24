/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "client.h"

int main(int argc, char *argv[])
{    
    if (argv[1] == NULL) 
    {
		printf("Error! Incorrect argument.\n\n");
		printf("Usage: client [host name]\n");
  		exit(EXIT_FAILURE);
    }
    char hostName[50];
    memset(&hostName, 0, sizeof(char));
    strncpy(hostName, argv[1], 50);
	hostName[50-1] = '\0';

    int sock = createSocket(CLIENTPORT);

	// Print startup time for demonstration
	time_t startupTime;
	time(&startupTime);
	printf("\nStarting client: %s\n", ctime(&startupTime));

    /*
        Begin connection attempt.
        Return value does not need to be checked as program will
        close upon error.
    */
    ConnectionInfo serverInfo = connectToServer(sock, hostName);

	fd_set activeFdSet, readFdSet;
	FD_ZERO(&activeFdSet);
	FD_ZERO(&readFdSet);
	FD_SET(STDIN_FILENO, &activeFdSet);
	FD_SET(serverInfo.sock, &activeFdSet);

	char message[MESSAGELENGTH] = { '\0' };

	//todo: to match with our labb 3a report.
	int nextSeq = serverInfo.baseSeqNum;
	int retval = 0;
	printCursorThingy();
	fflush(stdout);
	clock_gettime(CLOCK_MONOTONIC_RAW, &serverInfo.buffer[nextSeq].timeStamp);
	struct timeval t;
	t.tv_sec = 1;
	t.tv_usec = 0;
	while(1)
	{
		readFdSet = activeFdSet;
		checkTimedOutPacks(&serverInfo, &nextSeq);
		
		if (select(FD_SETSIZE, &readFdSet, NULL, NULL, &t) < 0)
		{
			//perror("\nFailed to monitor set");
			//* FD_ZERO prevents reusing old set if select gets interrupted by timer
			FD_ZERO(&readFdSet);
			//printCursorThingy();
			//exit(EXIT_FAILURE);
		}

		for (int currSock = 0; currSock < FD_SETSIZE; currSock++)
		{
			/* 
				* Prioritize receiving from server to avoid blocking
				* ACKs through constant typing.
			*/
			if (currSock == serverInfo.sock && FD_ISSET(currSock, &readFdSet))
			{
				interpretPack_sender(&serverInfo, nextSeq);
				printCursorThingy();
			}
			else if (currSock == STDIN_FILENO && FD_ISSET(currSock, &readFdSet))
			{
				fgets(message, MESSAGELENGTH, stdin);
				message[MESSAGELENGTH - 1] = '\0';

				if (strncmp(message, "EXIT\n", MESSAGELENGTH) == 0)
					DisconnectClientSide(serverInfo, nextSeq);

				retval = writeMessage(&serverInfo, message, &nextSeq);
				if (retval == ERRORCODE) printf("Could not send message!\n");
				else if (retval == 0) printf("Window is full!\n");
				else nextSeq = (nextSeq + 1) % MAXSEQNUM;
				printCursorThingy();
			}
		}
		fflush(stdout);
	}
    return 0;
}

void printCursorThingy()
{
	printf("------------------------\n");
	printf("\n(Type \"EXIT\" to disconnect)\n\n\n>");
}

//? Move this when testing is done
int writeMessage(ConnectionInfo *server, char* message, int *nextSeq)
{
	int retval;
	if (SWMETHOD == GBN) retval = writeMessageGBN(server, message, *nextSeq);
	else retval = writeMessageSR(server, message, nextSeq);
	return retval;
}

int writeMessageGBN(ConnectionInfo *server, char* message, int nextSeq)
{
	// Don't send if window full
	for(int i = server->baseSeqNum, count = 0; i != nextSeq; i = (i+1) % MAXSEQNUM, count++)
		if (count >= WINDOWSIZE) return 0;

	Datagram toSend = initDatagram();
	packMessage(toSend, message, nextSeq);
	if (sendMessage(server->sock, toSend, server->addr) < 0) return ERRORCODE;

	// Add message to buffer and move window
	strncpy(server->buffer[nextSeq].message, message, strlen(message));
	clock_gettime(CLOCK_MONOTONIC_RAW, &server->buffer[nextSeq].timeStamp);

	// Print timestamp
	time_t currTime;
	time(&currTime);
	printf("Message sent at: %s", ctime(&currTime));
	printf("-with SEQ(%d)\n", toSend->seqNum);
	return 1;
}

int writeMessageSR(ConnectionInfo *server, char* message, int* nextSeq)
{
	Datagram toSend = initDatagram();
	packMessage(toSend, message, *nextSeq);
	time_t currTime;
	time(&currTime);
	
	/*
	for (int i = server->baseSeqNum, count = 0;  == 0; i = ((i+1) % MAXSEQNUM), count++)
	{
		if (server->buffer[i+1].message[0] == '\0')
			printf("Count is: %d", count);
		if (count > WINDOWSIZE) return 0;
	}*/

	for(int i = server->baseSeqNum, count = 0; i != *nextSeq; i = (i+1) % MAXSEQNUM, count++)
		if (count >= WINDOWSIZE) return 0;
		
	
	if (sendMessage(server->sock, toSend, server->addr) < 0) return ERRORCODE;
    printf("Message sent at: %s", ctime(&currTime));
	printf("-with SEQ(%d)\n", toSend->seqNum);

	strncpy(server->buffer[*nextSeq].message, message, strlen(message));
	clock_gettime(CLOCK_MONOTONIC_RAW, &server->buffer[*nextSeq].timeStamp);
    //* Start TIMER
        
	return 1;
}

void interpretPack_sender(ConnectionInfo *server, int nextSeq)
{
	Datagram receivedDatagram = initDatagram();
	recvMessage(server->sock, receivedDatagram, &server->addr);

	//* Send to GBN or SR to handle DATA in package
	if (SWMETHOD == GBN) interpretPack_sender_GBN(receivedDatagram, server);
	else interpretPack_sender_SR(receivedDatagram, server, nextSeq);
}

void interpretPack_sender_GBN(Datagram receivedDatagram, ConnectionInfo *server)
{
	int isCorrupt = corrupt(receivedDatagram);
	if (receivedDatagram->flag == ACK && !isCorrupt && receivedDatagram->ackNum == server->baseSeqNum)
	{
		printf("\nReceived ACK(%d)\n\n", receivedDatagram->ackNum);

		//* Reset data on buffer-spot and move window
		server->buffer[receivedDatagram->ackNum].timeStamp.tv_sec = 0;
		server->buffer[receivedDatagram->ackNum].timeStamp.tv_nsec = 0;
		for (int i = 0; i < MESSAGELENGTH; i++)
			server->buffer[receivedDatagram->ackNum].message[i] = '\0';
		// server->baseSeqNum = receivedDatagram->ackNum;
		server->baseSeqNum = (server->baseSeqNum + 1) % MAXSEQNUM;
	}
	else printf("\nReceived a corrupt packet!\n");
}

void interpretPack_sender_SR(Datagram receivedDatagram, ConnectionInfo* server, int nextSeq)
{
	time_t currTime;
	time(&currTime);
	int isCorrupt = corrupt(receivedDatagram);
	if((receivedDatagram->flag == ACK) && !isCorrupt)
	{
		printf("Received Datagram at: %s", ctime(&currTime));
		printf("-With ACK(%d)\n", receivedDatagram->ackNum);
		memset(&server->buffer[receivedDatagram->ackNum], 0, sizeof(server->buffer[receivedDatagram->ackNum]));
		// server->buffer[receivedDatagram->ackNum].timeStamp.tv_sec = 0;
		// server->buffer[receivedDatagram->ackNum].timeStamp.tv_nsec = 0;
		// for (int i = 0; i < MESSAGELENGTH; i++)
		// 	server->buffer[receivedDatagram->ackNum].message[i] = '\0';

		printf("ackNum %d| baseSeq %d\n", receivedDatagram->ackNum, server->baseSeqNum);
		if (receivedDatagram->ackNum == server->baseSeqNum)
		{
			for (int i = server->baseSeqNum; i != nextSeq; i= ((i+1) % MAXSEQNUM))
			{
				if (server->buffer[i].timeStamp.tv_sec == 0) 
				{
					server->baseSeqNum = (server->baseSeqNum + 1) % MAXSEQNUM;
				}
			}
		}
		else if (receivedDatagram->ackNum == 0) server->baseSeqNum = receivedDatagram->ackNum;
			
		
	}
	else if (isCorrupt)
	{
		printf("Received corrupt package\n");
	} 
}

void checkTimedOutPacks(ConnectionInfo *server, int *nextSeq)
{
	/*
		Simple redirecting switch as they are handled different depending
		on which sliding window is to be used.
	*/
	if (SWMETHOD == GBN) checkTimeout_GBN(server, nextSeq);
	else checkTimeout_SR(server, nextSeq);
}

void checkTimeout_GBN(ConnectionInfo *server, int *nextSeq)
{
	struct timespec currTime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &currTime);
	if (currTime.tv_sec - server->buffer[server->baseSeqNum].timeStamp.tv_sec > 2 * RTT)
	{
		
		// printf("\nSending timed out packet\n");
		// writeMessageGBN(server, server->buffer[server->baseSeqNum].message, server->baseSeqNum);
		// clock_gettime(CLOCK_MONOTONIC_RAW, &server->buffer[server->baseSeqNum].timeStamp);
	
		for (int seq = (server->baseSeqNum); seq != *nextSeq; seq = (seq+1) % MAXSEQNUM)
		{
			printf("\nSending timed out packet\n");
			writeMessageGBN(server, server->buffer[seq].message, seq);
			clock_gettime(CLOCK_MONOTONIC_RAW, &server->buffer[seq].timeStamp);
		}
	
	}
}

void checkTimeout_SR(ConnectionInfo *server, int *nextSeq)
{
	struct timespec currTime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &currTime);

	for (int seq = (server->baseSeqNum); seq != *nextSeq; seq = (seq+1) % MAXSEQNUM)
	{
		if(currTime.tv_sec - server->buffer[seq].timeStamp.tv_sec > 2 * RTT)
		{
			if (server->buffer[seq].timeStamp.tv_sec == 0) continue;
			printf("\nSending timed out packet\n");
			writeMessageSR(server, server->buffer[seq].message, &seq);
			clock_gettime(CLOCK_MONOTONIC_RAW, &server->buffer[seq].timeStamp);
		}
	}
}

void selectTimeout(int signal)
{
	return;
}