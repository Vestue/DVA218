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
    int sock = createSocket(PORT);
    struct sockaddr_in destAddr;
	
    if (argv[1] == NULL) 
    {
       perror("Usage: client [host name]\n");
       exit(EXIT_FAILURE);
    }
    char hostName[50];
    memset(&hostName, 0, sizeof(char));
    strncpy(hostName, argv[1], 50);
	hostName[50-1] = '\0';

    /*
        Begin connection attempt.
        Return value does not need to be checked as program will
        close upon error.
    */
    int currentSeq = setupConnection(sock, hostName, &destAddr);
    printf("%d\n", currentSeq);
    return 0;
}

int setupConnection(int sock, char* hostName, struct sockaddr_in* destAddr)
{
    // Setup destination adress.
    struct hostent *hostInfo;
    hostInfo = gethostbyname(hostName);
    destAddr->sin_family = AF_INET;
    destAddr->sin_port = htons(SERVERPORT);
    destAddr->sin_addr = *(struct in_addr *)hostInfo->h_addr;

    // Setup first message to be sent.
    Datagram messageToSend = initDatagram();
    messageToSend->header.flag = SYN;

    //! Test message, remove later
    char * msg = "Banana!\0";
    strncpy(messageToSend->message, msg, strlen(msg));

    // Attempt handshake with server
    if (connectToServer(sock, messageToSend, *destAddr) != 1)
    {
        printf("Failed connection handshake.");
        exit(EXIT_FAILURE);
    }
    return messageToSend->header.sequence;
}

int connectToServer(int sock, Datagram connRequest, struct sockaddr_in dest)
{

	//sätter flaggan till SYN och skickar, startar timer på 2 sek
	connRequest->header.flag = SYN;
	if(sendMessage(sock, connRequest, dest) < 0)
	{
		perror("Could not send message to server");
		exit(EXIT_FAILURE);
	}

    Datagram messageToReceive = initDatagram();
    struct sockaddr_in recvAddr;

	while(1)
	{
		signal(SIGALRM, timeoutTest);
		alarm(2);

        if (recvMessage(sock, messageToReceive, &recvAddr))
        {
            printf("\nReceived from server: ");
            puts(messageToReceive->message);
            printf("%d", messageToReceive->header.flag);
        }

		if(messageToReceive->header.flag == SYN + ACK)
		{
            connRequest->header.sequence += 1;
			connRequest->header.flag = ACK;
			if(sendMessage(sock, connRequest, dest) == 0)
			{
				perror("Could not send message to server");
				exit(EXIT_FAILURE);
			}
			printf("Connection established");

			return 1;
		}
	}
	return 0;
}