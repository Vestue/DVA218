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

int sendMessage(int sock, Datagram messageToSend, struct sockaddr_in destAddr)
{

}

void setDefaultHeader(Datagram messageToSend)
{
	messageToSend->header.windowSize = WINDOWSIZE;
	messageToSend->header.sequence = 1;
	messageToSend->header.flag = UNSET;
	messageToSend->message = '\0';
}

int createSocket(int port)
{
	int sock;
	struct sockaddr_in addr;

	if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0)
	{
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sock, (struct sockaddr_in*)&addr, sizeof(addr)) < 0)
	{
	   perror("Could not bind socket!");
	   exit(EXIT_FAILURE);
	}
	return sock;
}

int connect(int sock, Datagram connectionReq, struct sockaddr_in dest)
{

    //sätter flaggan till SYN och skickar, startar timer på 2 sek
    connectionReq->header.flag = SYN;
    if(sendMessageToServer(sock, connectionReq) < 0)
    {
        perror("Could not send message to server");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        signal(SIGALRM, timeoutConnection(sock, connectionReq, dest));
        alarm(2);
        if(connectionReq->header.flag = SYNACK)
        {      
            connectionReq->header.flag = ACK;
            if(sendMessageToServer(sock, connectionReq, dest) < 0)
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

, struct sockaddr_in destint connect(int sock, Datagram connectionReq)
, struct sockaddr_in dest{

    //sätter flaggan till SYN och skickar, startar timer på 2 sek
    connectionReq->header.flag = SYN;
, dest(sendMessageToServer(sock, connectionReq) < 0), dest
    {
        perror("Could not send message to server");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
      signal(SIGALRM, timeoutConnection(sock,cconnectionReq, dest));
        alarm(2);
        if(connectionReq->header.flag = SYNACK)
        {      
            connectionReq->header.flag = ACK;
            if(sendMessageToServer(sock, connectionReq, dest) < 0)
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

, struct sockaddr_in destn, Datagram connectionReq)
, struct sockaddr_in dest{
	
    while(1)
    {

        switch(connectionReq->header.flag)

connec            cas
      connectioe SYN:nReq->header.flag; 
            e SYN: sendMessageToServer(sock,tionReq, dest ;
            caseignal(SIGALRM, timeout); alarm( connec2); break;
)
            case ACK: printf("Connection Established"); return 1;)

    }


    return 0;}

void timeoutConnection(int sock, Datagram connectionReq)
{
, Datastruct sockaddr_in dest, struct sockaddr_in dest    connectioReq->header.flag = SYN;

    if(sendMessageToServer(sock, connect, destionReq) < 0)
    {
, dest        perror("Could not ssage to server");
        exit(EXIT_FAILURE);
    }
}void timeoutAccept(){}return 1;    printf()""Connection established;    printf()""COnneconnection established;return 1;