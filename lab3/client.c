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

    /*
        Begin connection attempt.
        Return value does not need to be checked as program will
        close upon error.
    */
    ConnectionInfo serverInfo = connectToServer(sock, hostName);
	serverInfo.baseSeqNum = serverInfo.baseSeqNum % MAXSEQNUM;
	
    printf("%d\n", serverInfo.baseSeqNum);

	fd_set activeFdSet, readFdSet;
	FD_ZERO(&activeFdSet);
	FD_ZERO(&readFdSet);
	FD_SET(STDIN_FILENO, &activeFdSet);
	FD_SET(serverInfo.sock, &activeFdSet);

	char message[MESSAGELENGTH] = { '\0' };
	int currentSeq = serverInfo.baseSeqNum;
	int currentWindow = serverInfo.windowCount;
	int retval = 0;
	printf("\n>");
	fflush(stdout);
	while(1)
	{
		//? Move this into the part where window gets moved
		serverInfo.baseSeqNum = serverInfo.baseSeqNum % MAXSEQNUM;
		readFdSet = activeFdSet;
		if (select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0)
		{
			perror("\nFailed to monitor set");
			//* FD_ZERO prevents reusing old set if select gets interrupted by timer
			FD_ZERO(&readFdSet);
			printf("\n>");
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
				if (retval == 1) printf("\nMessage sent!\n");
				else if (retval == ERRORCODE) printf("Could not send message!\n");
				else printf("Window is full!\n");
				printf("\n>");
			}
		}
		fflush(stdout);
	}

    //setupClientDisconnect(sock, hostName, &destAddr);
    return 0;
}

//? Move this when testing is done
int writeMessage(ConnectionInfo *server, char* message, int* currentSeq)
{
	int retval;
	if (SWMETHOD == SR) retval = writeMessageGBN(server, message, currentSeq);
	else retval = writeMessageSR(server, message, currentSeq);
	return retval;
}

int writeMessageGBN(ConnectionInfo *server, char* message, int* currentSeq)
{
	// Don't send if window full
	if(abs(server->baseSeqNum - *currentSeq) > WINDOWSIZE) return 0;

	Datagram toSend = initDatagram();
	packMessage(toSend, message, *currentSeq);
	if (sendMessage(server.sock, toSend, server.addr) < 0) return ERRORCODE;
	*currentSeq = (*currentSeq + 1) % MAXSEQNUM;
	return 1;
}



	if (sendMessage(server.sock, toSend, server.addr) < 0) return ERRORCODE;
	*currentSeq = (*currentSeq + 1) % MAXSEQNUM;
	return 1;

