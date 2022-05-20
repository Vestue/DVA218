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
#define UNSET 0
#define SYN 1
#define ACK 2
#define FIN 4

// TODO: Fix checksum function

// int calcChecksum(const Datagram toCalc)
// {
//     uint8_t P = 0b100000111;
//     int n = 9, w = 8;
//     uint len = sizeof(*toCalc);
//     for (int i = 0; i < len; i++)
//     {
//         if ()
//     }
    
//     return 0;
// }

int recvMessage(int sock, Datagram receivedMessage, struct sockaddr_in* receivedAdress)
{
	struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    unsigned int addrlen = sizeof(recvAddr);

    if (recvfrom(sock, (Datagram)receivedMessage, sizeof(Header),
        0, (struct sockaddr *)&recvAddr, &addrlen) < 0) 
    {
        perror("Error receiving message!\n");
		exit(EXIT_FAILURE);
    }
    *receivedAdress = recvAddr;
    return 1;
}

int sendMessage(int sock, Datagram messageToSend, struct sockaddr_in destAddr)
{
	if (sendto(sock, (Datagram)messageToSend, sizeof(Header),
	    0, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0)
    {
        perror("Failed to send message\n");
        return 0;
    }
    return 1;
}

void setDefaultHeader(Datagram messageToSend)
{
	messageToSend->windowSize = WINDOWSIZE;
	messageToSend->sequence = 1;
	messageToSend->flag = UNSET;
	messageToSend->message[0] = '\0';
}

Datagram initDatagram()
{
    Datagram temp = (Datagram)calloc(1 , sizeof(Header));
    if (temp == NULL)
    {
        perror("Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    setDefaultHeader(temp);
    return temp;
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

int createClientSpecificSocket(struct sockaddr_in clientAddr)
{
	int sock;
	struct sockaddr_in addr;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
    memset(&addr, 0, sizeof(addr));
	addr = clientAddr;
    addr.sin_port = htons(CLIENTPORT);
	
	if (bind(sock, (struct sockaddr*)&addr, (unsigned int)sizeof(addr)) < 0)
	{
	   perror("Could not bind socket!");
	   exit(EXIT_FAILURE);
	}
	return sock;
}

void timeoutTest()
{
    printf("\nTimed out\n");

} 


void timeoutConnection(int sock, Datagram connRequest, struct sockaddr_in dest)
{
	connRequest->flag = SYN;

    if(sendMessage(sock, connRequest, dest) < 0)
    {
        perror("Could not send message to server");
        exit(EXIT_FAILURE);
    }
}

int connectToServer(int sock, char* hostName, struct sockaddr_in* destAddr)
{
    // Setup destination adress.
    struct hostent *hostInfo;
    hostInfo = gethostbyname(hostName);
    destAddr->sin_family = AF_INET;
    destAddr->sin_port = htons(SERVERPORT);
    destAddr->sin_addr = *(struct in_addr *)hostInfo->h_addr;

    // Setup first message to be sent.
    Datagram messageToSend = initDatagram();
    setHeader(messageToSend, SYN, NULL);

    //! Test message, remove later
    char * msg = "Banana!\0";
    strncpy(messageToSend->message, msg, strlen(msg));

    // Attempt handshake with server
    if (initHandshakeWithServer(sock, messageToSend, *destAddr) != 1)
    {
        printf("Failed connection handshake.\n");
        exit(EXIT_FAILURE);
    }
    return messageToSend->sequence;
}

int initHandshakeWithServer(int sock, Datagram connRequest, struct sockaddr_in dest)
{
	if(sendMessage(sock, connRequest, dest) < 0)
	{
		perror("Could not send message to server.\n");
		exit(EXIT_FAILURE);
	}

    Datagram messageToReceive = initDatagram();
    struct sockaddr_in recvAddr;

	while(1)
	{
		signal(SIGALRM, timeoutTest);
		alarm(2);

        if (recvMessage(sock, messageToReceive, &recvAddr))
        {
            printf("\nReceived from server: ");
            puts(messageToReceive->message);
            printf("%d", messageToReceive->flag);
        }

		if(messageToReceive->flag == SYN + ACK)
		{
            setHeader(connRequest, ACK, messageToReceive);
			if(sendMessage(sock, connRequest, dest) == 0)
			{
				perror("Could not send message to server\n");
				exit(EXIT_FAILURE);
			}
			printf("Connection established\n");

			return 1;
		}
	}
	return 0;
}

int acceptClientConnection(int sock, Datagram connRequest, struct sockaddr_in* dest, ClientList* list)
{
    struct sockaddr_in tempAddr;
	Datagram toSend = initDatagram();

	/*	Socket is created here to avoid creating several sockets
		if the SYN+ACK gets lost.
	*/
	int clientSock = createClientSpecificSocket(*dest);
	while(1)
	{		
		if (connRequest->flag == SYN)
		{
            *dest = tempAddr;
			connRequest->flag = SYN + ACK;
			printf("Received SYN\n");
			setHeader(toSend, SYN + ACK, connRequest);
			signal(SIGALRM, timeoutTest);
			alarm(2);

			// Send using the clientSock so client gets address of
			// designated port.
			if(sendMessage(clientSock, toSend, *dest) == 0)
			{
			    perror("Could not send message to client\n");
			    exit(EXIT_FAILURE);
			}
		}
        else	// First message from a new client must be SYN
		{
			free(toSend);
			close(clientSock);
			return 0; 
		} 

        /*
            Wait for ACK from the expected address.
        ?   Might need to move this as the connection will be blocked if 
        ?   every ACK gets lost from this client.

            SYN could arrive here if the SYN+ACK got lost,
            if so then the server will resend SYN+ACK and then wait here again.
        */
        recvMessage(sock, connRequest, &tempAddr);
		
        //* Make sure that ACK is coming from expected adress
    	if(connRequest->flag == ACK 
            && tempAddr.sin_addr.s_addr == dest->sin_addr.s_addr
            && tempAddr.sin_port == dest->sin_port)
        {
            printf("\nConnection established\n");
            return 1;
			if (addToClientList(list, initConnectionInfo(connRequest, *dest, clientSock)))
			{
				printf("Connection established\n");
				free(toSend);
            	return 1;
			}
			free(toSend);
			close(clientSock);
            return ERORRCODE;
        }
	}
}

void interpretPack_receiver(int sock, Datagram packet, struct sockaddr_in addr, ClientList *clients)
{
    

	if (SWMETHOD == GBN) interpretWith_GBN_receiver(sock, packet, addr, clients);
	else interpretWith_SR_receiver(sock, packet, addr, clients);
}

void interpretWith_GBN_receiver(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients)
{
	Datagram messageToSend = initDatagram();
    printf("%d", messageToSend->windowSize); //? Just to get rid of warnings
}

void interpretWith_SR_receiver(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients)
{
	Datagram messageToSend = initDatagram();
    printf("%d", messageToSend->windowSize); //? Just to get rid of warnings
}




/* List functions */

ClientList initClientList()
{
    ClientList list;
    ConnectionInfo* tempArr = (ConnectionInfo*)calloc(1, sizeof(ConnectionInfo));
    if (tempArr == NULL)
    {
        perror("Failed to allocate memory for client list");
        exit(EXIT_FAILURE);
    }
    list.clients = tempArr;
    list.size = 0;
    return list;
}

ConnectionInfo initConnectionInfo(Datagram receivedDatagram, struct sockaddr_in recvAddr, int sock)
{
	ConnectionInfo tempInfo;
	if (receivedDatagram->flag == ACK) tempInfo.baseSeqNum = STARTSEQ;
	else if (receivedDatagram->flag == SYN + ACK) tempInfo.baseSeqNum = receivedDatagram->ackNum;
	else
	{
		printf("\nWrong usage of initConnectionInfo\n");
		exit(EXIT_FAILURE);
	}
	tempInfo.addr = recvAddr;
	tempInfo.FIN_SET = 0;
	tempInfo.sock = sock;
	return tempInfo;
}

int addToClientList(ClientList *list, ConnectionInfo info)
{
    int cur = list->size;
    list->size += 1;
    list->clients = realloc(list->clients, list->size * sizeof(ConnectionInfo));
    if (list->clients == NULL) return 0;
    list->clients[cur] = info;
    return 1;
}

int removeFromClientList(ClientList *list, struct sockaddr_in addr)
{
	if(list == NULL) return 0;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
        {
            /*
                * Make a temporary array to copy data to and then free and repoint old one.
                * This is done to make sure that the array only uses needed amount of memory
                * and don't contain any old entries.
            */
            ConnectionInfo* tempArr = (ConnectionInfo*)calloc(list->size, sizeof(ConnectionInfo));
            if (tempArr == NULL)
            {
                perror("Failed to allocate memory for client list");
                exit(EXIT_FAILURE);
            }
            // Copy everything before the index
            if (i != 0) memcpy(tempArr, list->clients, i * sizeof(ConnectionInfo));
            // Copy after index
            if (i != list->size - 1) 
                memcpy(tempArr + i, list->clients + i + 1, (list->size - i - 1) * sizeof(ConnectionInfo));
            free(list->clients);
            list->clients = tempArr;
            return 1;
        }
    }
    return 0;
}

int isInClientList(ClientList *list, struct sockaddr_in addr)
{
	if(list == NULL) return 0;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
            return 1;
    }
    return 0;
}

ConnectionInfo* findClient(ClientList *list, struct sockaddr_in addr)
{
	if(list == NULL) return NULL;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
            return &list->clients[i];
    }
    return NULL;
}

int setupClientDisconnect(int sock, char* hostName, struct sockaddr_in* destAddr)
{
    struct hostent* hostInfo;
    hostInfo = gethostbyname(hostName);
    destAddr->sin_family = AF_INET;
    destAddr->sin_port = htons(SERVERPORT);
    destAddr->sin_addr = *(struct in_addr *)hostInfo->h_addr;
    Datagram datagramToSend = initDatagram();

    if(DisconnectClientSide(sock, datagramToSend, *destAddr) != 1)
    {
        printf("Failed disconnect handshake");
        exit(EXIT_FAILURE);

    }

    return datagramToSend->sequence;
}

int DisconnectServerSide(int sock, Datagram disconnRequest, struct sockaddr_in* dest)
{

    Datagram receivedDatagram = initDatagram();
	struct sockaddr_in tempAddr;
    if(recvMessage(sock, receivedDatagram, &tempAddr) < 0)
    {
        printf("Failed to disconnect from client\n");
        exit(EXIT_FAILURE);
    }

    if(receivedDatagram->flag == FIN)
    {
        *dest = tempAddr;
        disconnRequest->flag = ACK;
        if(sendMessage(sock, disconnRequest, *dest) < 0)
        {
            printf("Failed to disconnect from client\n");
            exit(EXIT_FAILURE);
        }

        disconnRequest->flag = FIN;
    }
    

	while(1)
	{
        if (sendMessage(sock, disconnRequest, *dest) < 0)
        {
            printf("Failed to disconnect from client");
            exit(EXIT_FAILURE);
        }
        

        if(receivedDatagram->flag == ACK)
        {
            printf("\nDisconnected\n");

            return 1;

        }

	
        recvMessage(sock, receivedDatagram, &tempAddr);
	}


}

int DisconnectClientSide(int sock, Datagram disconnRequest, struct sockaddr_in destAddr)
{
    Datagram messageReceived = initDatagram();
    struct sockaddr_in tempAddr;
    int counter = 0;
	
    printf("In disconnect clientside\n");
	disconnRequest->flag = FIN;	
	signal(SIGALRM, timeoutTest);
	alarm(2);
	if(sendMessage(sock, disconnRequest, destAddr) < 0)
	{
		printf("Failed to disconnect from server");
		exit(EXIT_FAILURE);
	}
 

	while(1)
	{

		recvMessage(sock, messageReceived, &tempAddr);

		if(messageReceived->flag == FIN && counter == 0)
		{
           
			counter++;
			disconnRequest->flag = ACK;
			sendMessage(sock, disconnRequest, destAddr);
			signal(SIGALRM, timeoutTest);
			alarm(2); //Ska vara 2 * MSL

		}
		if(messageReceived->flag == FIN && counter > 0) //if counter is more than 0 that means it is the second FIN received
		{
			disconnRequest->flag = ACK;
            printf("\nDisconnected\n");
			sendMessage(sock, disconnRequest, destAddr);
			alarm(2); //resetar timern, går timern ut är clienten disconnectad
            
			return 1; //temporär return
		}
		
		if(messageReceived->flag == ACK)
            counter++;
           
           
            
        
			
		
		
		
	
			
	}

}

/*
    * Functions to get values from ConnectionInfo
*/

int getExpectedSeq(struct sockaddr_in addr, ClientList* list)
{
	if(list == NULL) return ERORRCODE;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
            return list->clients[i].baseSeqNum;
    }
    // Return ERRORCODE as it can't find client.
    return ERORRCODE;
}

int setBaseSeq(int seqToSet, struct sockaddr_in addr, ClientList* list)
{
	if(list == NULL) return ERORRCODE;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
        {
            list->clients[i].baseSeqNum = seqToSet;
            return 1;
        }
    }
    return ERORRCODE;
}

int setFIN(struct sockaddr_in addr, ClientList* list)
{
	if(list == NULL) return ERORRCODE;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
        {
            list->clients[i].FIN_SET = 1;
            return 1;
        }
    }
    return ERORRCODE;
}

int isFINSet(struct sockaddr_in addr, ClientList* list)
{
	if(list == NULL) return ERORRCODE;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
        {
            return list->clients[i].FIN_SET;
        }
    }
    return ERORRCODE;
}

/*
    * Functions to set neccessary information in header depending on flag.
*/

void setHeader(Datagram datagramToSend, int flag, Datagram receivedDatagram)
{
    datagramToSend->windowSize = WINDOWSIZE;
	datagramToSend->message[0] = '\0';
    
    switch (flag)
    {
        case SYN:
            datagramToSend->flag = SYN;
            datagramToSend->sequence = MAXSEQNUM - 1;
            datagramToSend->ackNum = 0;
            break;
        case ACK:
            datagramToSend->flag = ACK;
            datagramToSend->ackNum = receivedDatagram->sequence + 1;
            break;
        case (SYN + ACK):
            datagramToSend->flag = SYN + ACK;
            datagramToSend->ackNum = receivedDatagram->sequence + 1;
            datagramToSend->sequence = STARTSEQ;
            break;
        case FIN:
            datagramToSend->flag = FIN;
            datagramToSend->sequence = receivedDatagram->ackNum;
            datagramToSend->ackNum = receivedDatagram->sequence + 1;
            break;
        default:
            datagramToSend->flag = UNSET;
            datagramToSend->sequence = receivedDatagram->ackNum;
            datagramToSend->ackNum = receivedDatagram->sequence + 1;
            break;
    }
}

void packMessage(Datagram datagramToSend, char* messageToSend, Datagram receivedDatagram)
{
    setHeader(datagramToSend, UNSET, receivedDatagram);
    strncpy(datagramToSend->message, messageToSend, strlen(messageToSend));
}