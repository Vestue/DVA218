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

	//todo: currentSeq will probably need to be renamed to nextSeqNum
	//todo: to match with our labb 3a report. 
	int currentSeq = serverInfo.baseSeqNum;
	int retval = 0;
	printCursorThingy();
	fflush(stdout);
	while(1)
	{
		//! Move this into the part where window gets moved
		serverInfo.baseSeqNum = serverInfo.baseSeqNum % MAXSEQNUM;
		readFdSet = activeFdSet;

		resendTimedOutPacks(&serverInfo, &currentSeq);

		if (select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0)
		{
			perror("\nFailed to monitor set");
			//* FD_ZERO prevents reusing old set if select gets interrupted by timer
			FD_ZERO(&readFdSet);
			printCursorThingy();
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
				//interpretPack_sender
			}
			else if (currSock == STDIN_FILENO && FD_ISSET(currSock, &readFdSet))
			{
				fgets(message, MESSAGELENGTH, stdin);
				message[MESSAGELENGTH - 1] = '\0';
				retval = writeMessage(&serverInfo, message, &currentSeq);
				if (retval == ERRORCODE) printf("Could not send message!\n");
				else if (retval == 0) printf("Window is full!\n");
				else currentSeq = (currentSeq + 1) % MAXSEQNUM;
				printCursorThingy();
			}
		}
		fflush(stdout);
	}

    //setupClientDisconnect(sock, hostName, &destAddr);
    return 0;
}

void printCursorThingy()
{
	printf("\n(Type \"EXIT\" to disconnect)\n\n\n>");
}

//? Move this when testing is done
int writeMessage(ConnectionInfo *server, char* message, int *currentSeq)
{
	int retval;
	if (SWMETHOD == GBN) retval = writeMessageGBN(server, message, *currentSeq);
	else retval = writeMessageSR(server, message, currentSeq);
	return retval;
}

int writeMessageGBN(ConnectionInfo *server, char* message, int currentSeq)
{
	// Don't send if window full
	if(currentSeq > (server->baseSeqNum + WINDOWSIZE) % MAXSEQNUM) return 0;

	Datagram toSend = initDatagram();
	packMessage(toSend, message, currentSeq);
	if (sendMessage(server->sock, toSend, server->addr) < 0) return ERRORCODE;

	// Add message to buffer and move window
	strncpy(server->buffer[currentSeq].message, message, strlen(message));
	clock_gettime(CLOCK_MONOTONIC_RAW, &server->buffer->timeStamp);

	// Print timestamp for labspec
	time_t currTime;
	time(&currTime);
	printf("Message sent at: %s", ctime(&currTime));
	printf("-with SEQ(%d)\n", toSend->sequence);
	return 1;
}

void interpretPack_sender(ConnectionInfo *server, int *currentSeq)
{
	printf("In interpret sender!\n");
	Datagram receivedDatagram = initDatagram();
	recvMessage(server->sock, receivedDatagram, &server->addr);

	//* Send to GBN or SR to handle DATA in package
	if (SWMETHOD == GBN) interpretPack_sender_GBN(receivedDatagram, server);
	else interpretPack_sender_SR(receivedDatagram, server);
}

void interpretPack_sender_GBN(Datagram receivedDatagram, ConnectionInfo *server)
{

}

void interpretPack_sender_SR(Datagram receivedDatagram, ConnectionInfo *server)
{
	
}

void resendTimedOutPacks(ConnectionInfo *server, int *currentSeq)
{
	/*
		Simple redirecting switch as they are handled different depending
		on which sliding window is to be used.
	*/
	if (SWMETHOD == GBN) resendTimedOutPacks_GBN(server, currentSeq);
	else resendTimedOutPacks_SR(server, currentSeq);
}

void resendTimedOutPacks_GBN(ConnectionInfo *server, int *currentSeq)
{
	struct timespec currTime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &currTime);
	if (currTime.tv_sec - server->buffer[server->baseSeqNum].timeStamp.tv_sec > 2 * RTT)
	{
		for (int seq = server->baseSeqNum; seq < *currentSeq; seq++)
		{
			writeMessageGBN(server, server->buffer[seq].message, seq);
		}
	}
}

void resendTimedOutPacks_SR(ConnectionInfo *server, int *currentSeq)
{

}