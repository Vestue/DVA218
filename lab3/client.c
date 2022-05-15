/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "client.h"

//#define MAXLENGTH 512
#define PORT 5555

int main(int argc, char *argv[])
{
    printf("Yoo begin client\n");
    
    int sock;
    int currentSeq = 0;
    //Datagram receivedMessage;
	Datagram messageToSend;
    Datagram temp = (Datagram)calloc(1 , sizeof(Datagram));
    if (temp == NULL)
    {
        perror("Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    messageToSend = temp;
    printf("Before setDefaultHeader\n");
    char* msg = "Banana";
    setDefaultHeader(&messageToSend);
    
    messageToSend->header.flag = SYN;
    struct sockaddr_in destAddr;
    struct hostent *hostInfo;
    char *hostName;
    memset(&hostName, 0, sizeof(char));
    printf("Just before argv\n");

    if (argv[1] == NULL) 
    {
       perror("Usage: client [host name]\n");
       exit(EXIT_FAILURE);
    }
    printf("Made it through argv \n");

    strncpy(hostName, argv[1], 50);
	hostName[50-1] = '\0';
    hostInfo = gethostbyname(hostName);
    printf("Made it to destAddr :)\n");
    
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(PORT);
    destAddr.sin_addr = *(struct in_addr *)hostInfo->h_addr;
    printf("Made it through destAddr :)\n");

    if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0)
	{
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
    printf("Just before sending message :)\n");

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