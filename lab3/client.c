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
        Pointers are used as their values need to be used by other functions later on.
    */
    int currentSeq = setupConnection(&sock, hostName, &destAddr);
    printf("%d\n", currentSeq);
    return 0;
}

/*
    Setup information required for connection, go through the connection handshake
    with the server.
    Return sequence number sent in the server SYN+ACK.
*/
int setupConnection(int* sock, char* hostName, struct sockaddr_in* destAddr)
{
    //*sock = createSocket(PORT);

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
    if (connectToServer(*sock, messageToSend, *destAddr) != 1)
    {
        printf("Failed connection handshake.");
        exit(EXIT_FAILURE);
    }
    return messageToSend->header.sequence;
}