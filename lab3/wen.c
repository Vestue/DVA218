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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include "wen.h"

/*
	These macros need to be here instead of in the header
	as they otherwise cause issues with the enums using in the
	packet header.
*/
#define SYN 1
#define ACK 2
#define FIN 3

int recvMessage(int sock, Datagram receivedMessage)
{
	struct sockaddr_in addr;
	unsigned int addrlen = sizeof(addr);
	int msgLength;
    msgLength = recvfrom(sock, receivedMessage, sizeof(*receivedMessage), 0, (struct sockaddr *)&addr, (unsigned int*)&addrlen);
    if (msgLength < 0) {
        perror("Error receiving message!");
		exit(EXIT_FAILURE);
    } else if (msgLength == 0)
        return 0;
	return 1;
}

int sendMessage(int sock, Datagram messageToSend, struct sockaddr_in destAddr)
{
	/*	TODO 
		Pack packet with right flags and then send it
	*/
	sendto(sock, (Datagram)&messageToSend, sizeof(messageToSend),
	0, (struct sockaddr *)&destAddr, sizeof(destAddr));
}

void setDefaultHeader(Datagram messageToSend)
{
	messageToSend->header.flag = UNSET;
    messageToSend->header.sequence = 1;
    
	messageToSend->message = '\0';
}

int createSocket(int port)
{
	int sock;
	struct sockaddr_in addr;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sock, (struct sockaddr*)&addr, (unsigned int)sizeof(addr)) < 0)
	{
	   perror("Could not bind socket!");
	   exit(EXIT_FAILURE);
	}
	return sock;
}

int connectToServer(int sock, Datagram connRequest, struct sockaddr_in dest)
{

	//sätter flaggan till SYN och skickar, startar timer på 2 sek
	connRequest->header.flag = SYN;
	if(sendMessage(sock, connRequest, dest) < 0)
	{
		perror("Could not send message to server");
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		signal(SIGALRM, timeoutTest);
		alarm(2);
		if(connRequest->header.flag = SYNACK)
		{
			connRequest->header.flag = ACK;
			if(sendMessage(sock, connRequest, dest) < 0)
			{
				perror("Could not send message to server");
				exit(EXIT_FAILURE);
			}
			printf("Connection established");

			return 1;
		}
	}

	return 0;
}

int acceptConnection(int sock, Datagram connRequest, struct sockaddr_in dest)
{
	
	while(1)
	{
		if (connRequest->header.flag == SYN)
		{
			connRequest->header.flag = SYNACK;
			signal(SIGALRM, timeoutTest);
			alarm(2);
		}
        else if(connRequest->header.flag == ACK)
        {
            printf("Connection established");
            return 1;
        }
	}

}


void timeoutTest()
{
    printf("Timed out");

} 


void timeoutConnection(int sock, Datagram connRequest, struct sockaddr_in dest)
{
	connRequest->header.flag = SYN;

    if(sendMessage(sock, connRequest, dest) < 0)
    {
        perror("Could not send message to server");
        exit(EXIT_FAILURE);
    }
}