/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include "server.h"

int main() 
{
	int sock = createSocket();
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

					break;
				case ACK:
					// Chill timer
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
		}


		if (sequenceNumber >= MAXSEQNUM)
			sequenceNumber = 0;
	}

	return 0;
}
    
int createSocket()
{
	int sock;
	struct sockaddr_in addr;

	if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0)
	{
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sock, (struct sockaddr_in*)&addr, sizeof(addr)) < 0)
	{
	   perror("Could not bind socket!");
	   exit(EXIT_FAILURE);
	}
	return sock;
}

int sendMessageToClient(int sock, Datagram messageToSend)
{

	return 1;
}
int recvMessageFromClient(int sock, Datagram receivedMessage)
{

	return 1;
}

// TODO: Fix checksum function

void interpretPack(Datagram packet)
{
	if (packet->header.flag == GBN) 
	{
		// GBN
	}
	else 
		// SR
}

void defaultiseMessage(Datagram messageToSend)
{
	messageToSend->header.windowSize = WINDOWSIZE;
	messageToSend->header.sequence = 1;
	messageToSend->header.flag = GBN;
	messageToSend->message = '\0';
}