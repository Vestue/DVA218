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
	Datagram receivedMessage = initDatagram();
    //Datagram receivedMessage = (Datagram)calloc(1, sizeof(Datagram));
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