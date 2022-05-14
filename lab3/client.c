/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "client.h"

#define MAXLENGTH 512
#define PORT 5555

int main(int argc, char* argv[])
{
	int sock = createSocket(PORT);
	int currentSeq = 0;
	Datagram receivedMessage;
	Datagram messageToSend;
	setDefaultHeader(messageToSend);
    messageToSend->message = "Banana";
    messageToSend->header.flag = SYN;
    struct sockaddr_in destAddr;
    struct hostent *hostInfo;
    char *hostName;
	if (argv[1] == NULL)
	{
		perror("Usage: client [host name]\n");
        exit(EXIT_FAILURE);
    }
    strncpy(hostName, argv[1], 50);
	hostName[50-1] = '\0';
    hostInfo = gethostbyname(hostName);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(PORT);
    destAddr.sin_addr = *(struct in_addr *)hostInfo->h_addr;

    if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0)
	{
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
    sendMessageToServer(sock, messageToSend, destAddr);
    return 0;
}

int sendMessageToServer(int sock, Datagram toSend, struct sockaddr_in destAddr)
{
    sendMessage(sock, toSend, destAddr);
    return 1;
}
int recvMessageFromServer(int socket, Datagram receivedMessage)
{
	return 1;
}