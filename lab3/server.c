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
	Datagram messageToSend = initDatagram();
    struct sockaddr_in receivedAdress;

    printf("Just before loop\n");
	while (1) 
	{
		if (recvMessage(sock, receivedMessage, &receivedAdress)) 
		{
            printf("I successfylly received!\n");

			puts(receivedMessage->message);
            printf("%d", receivedMessage->header.flag);
        }


		if (sequenceNumber >= MAXSEQNUM)
			sequenceNumber = 0;
	}

	return 0;
}



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