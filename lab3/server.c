/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "server.h"

int main() 
{
	int sock = createSocket(PORT);
	int sequenceNumber = 0;
	// Datagram receivedMessage = initDatagram();
    Datagram receivedMessage = (Datagram)calloc(1, sizeof(Datagram));
    struct sockaddr_in servAddr;
    if (receivedMessage == NULL)
    {
        perror("Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
	Datagram messageToSend = initDatagram();

    printf("Just before loop\n");
	while (1) 
	{
        if (recvMessage(sock, receivedMessage, &servAddr) < 0)
        {
            printf("I received flag %d!\n", receivedMessage->header.flag);
            
			puts(receivedMessage->message);
            printf("%d", receivedMessage->header.flag);
        }
		// if (recvMessageFromClient(sock, receivedMessage))
		// {
        //     printf("I received flag %d!\n", receivedMessage->header.flag);
            
		// 	puts(receivedMessage->message);
        //     printf("%d", receivedMessage->header.flag);
        // }

		if (sequenceNumber >= MAXSEQNUM) sequenceNumber = 0;
	}

	return 0;
}

int sendMessageToClient(int sock, Datagram messageToSend, struct sockaddr_in destAddr)
{
	signal(SIGALRM, sendTimeout);
	alarm(2);
	// om paketet skickas reseta timern genom att calla alarm(2) igen
	

	return 1;
}
int recvMessageFromClient(int sock, Datagram receivedMessage)
{
    // TODO This variable needs to be returned somehow
    // As we need to store the address of the client
	struct sockaddr_in recvAddr;

    unsigned int* addrlen = sizeof(recvAddr);
    int msgLength;
    printf("I am in messageclient now\n");
    if (msgLength = recvfrom(sock, (Datagram)&receivedMessage, sizeof(struct Packet),
        0, (struct sockaddr *)&recvAddr, &addrlen) < 0)
    {
        perror("Error receiving message!");
		exit(EXIT_FAILURE);
    } else if (msgLength == 0)
    {
        printf("message length is %d", msgLength);
        return 0;
    }
    printf("Got a message longer than 0!\n");
    return 1;
}

// TODO: Fix checksum function

void interpretPack(int sock, Datagram packet, Datagram messageToSend)
{
	if (packet->header.flag == GBN) 
	{
		// GBN
		
	}
	else
	{
		// SR
	}
		
}

void sendTimeout()
{
    printf("Timeout!");
}