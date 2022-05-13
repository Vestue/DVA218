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

int main()
{
    int sock = createSocket();
	int currentSeq = 0;
	Datagram receivedMessage;
	Datagram messageToSend;
	setDefaultHeader(messageToSend);
    struct sockaddr_in destAddr;

    if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0)
    {
        perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
    }

    return 0;
}


    

}

int sendMessageToServer(int sock, Datagram toSend, sockaddr_in destAddr)
{

	sendto(sock, (Datagram)&toSend, sizeof(toSend), 0, )
    return 1;
}
int recvMessageFromServer(int socket, Datagram receivedMessage)
{
    return 1;
}