/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "wen.c"
#include "server.h"

int main() 
{
	int sock = createSocket(PORT);
	int sequenceNumber = 0;
	Datagram receivedMessage;
	Datagram messageToSend;
	memset(receivedMessage, 0, sizeof(receivedMessage));
	memset(messageToSend, 0, sizeof(messageToSend));

	while (1) 
	{
        defaultiseMessage(messageToSend);

		if (recvMessageFromClient(sock, receivedMessage)) 
		{
            switch (receivedMessage->header.flag)
			{
				case SYN:
					// Send flag SYN+ACK
					messageToSend->header.flag = SYN + ACK;
                    sendMessageToClient(sock, messageToSend);
					// TODO Start timer
                    break;
				//! Can recieve ACKs, FINs, or data 
				//! before connection even has been attempted
				//? Check if client_addr is in client addr
                case ACK:
					// TODO Chill timer
					break;
				case FIN:
					/*kill*/
					break;
				default:
					/*Destroy and ACK old packages*/
					/*Normal packet*/
					messageToSend->header.flag = ACK;
					messageToSend->header.sequence = receivedMessage->header.sequence + 1;
					break;
			}
			// TODO: if SYN_timer_timeout
        }


		if (sequenceNumber >= MAXSEQNUM)
			sequenceNumber = 0;
	}

	return 0;
}

int sendMessageToClient(int sock, Datagram messageToSend, sockaddr_in destAddr)
{
	signal(SIGALRM, timeoutSend);
	alarm(2);
	// om paketet skickas reseta timern genom att calla alarm(5) igen
	

	return 1;
}
int recvMessageFromClient(int sock, Datagram receivedMessage)
{
	struct sockaddr_in addr;
    unsigned int addrlen = sizeof(addr);
    int msgLength;

    if (msgLength = recvfrom(sock, receivedMessage, sizeof(*receivedMessage),
        0, (struct sockaddr*)&addr, &addrlen < 0))
	{
		perror("Error receiving message!");
		exit(EXIT_FAILURE);
	}
	else if (msgLength == 0)
		return 0;
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