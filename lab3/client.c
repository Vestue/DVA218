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
    int sock;
    int currentSeq = 0;
    //Datagram receivedMessage;
	Datagram messageToSend = initDatagram();
    messageToSend->header.flag = SYN;
    char * msg = "Banana!\0";
    strncpy(messageToSend->message, msg, sizeof(msg));


    struct sockaddr_in destAddr;
    struct hostent *hostInfo;
    char hostName[50];
    memset(&hostName, 0, sizeof(char));

    if (argv[1] == NULL) 
    {
       perror("Usage: client [host name]\n");
       exit(EXIT_FAILURE);
    }

    strncpy(hostName, argv[1], 50);
	hostName[50-1] = '\0';
    hostInfo = gethostbyname(hostName);
    
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SERVERPORT);
    destAddr.sin_addr = *(struct in_addr *)hostInfo->h_addr;
    printf("Made it through destAddr :)\n");

    sock = createSocket(PORT);
    sendMessage(sock, messageToSend, destAddr);
    printf("Message sent\n");


    // * Attempt to receive
    Datagram messageToReceive = initDatagram();
    struct sockaddr_in recvAddr;
    if (recvMessage(sock, messageToReceive, &recvAddr))
    {
        printf("\nReceived from server: ");
        puts(messageToReceive->message);
        printf("%d", messageToReceive->header.flag);
    }

    scanf("Press enter to continue...");
    return 0;
}