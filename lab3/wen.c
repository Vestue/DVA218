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
#include <time.h>
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

uint32_t calcChecksum(const void* message, uint32_t length)
{
    uint32_t poly = 0x04C11DB7;
    const uint8_t* M8 = (const uint8_t*)message;
    uint32_t result = 0;
    for (uint32_t i = 0; i < length; ++i)
    {
        result ^= M8[i];
        for (uint32_t j = 0; j < 8; ++j)
        {
            result = result & 1 ? (result >> 1) ^ poly : result >> 1;
        }
    }
    return result;
}

int corrupt(Datagram toCheck)
{
	//Creates a copy of packet with a zero'd checksum
	Datagram zeroedChecksum = (Datagram)calloc(1, sizeof(*toCheck));
	memcpy(zeroedChecksum, toCheck, sizeof(*toCheck));
	zeroedChecksum->checksum = 0;
	zeroedChecksum->checksum = calcChecksum((Datagram)zeroedChecksum, (sizeof(*zeroedChecksum)));

	//? Add error inducing code here later
	// example: if rand<25 return 1
	if (toCheck->checksum == zeroedChecksum->checksum)
		return 0;
	return 1;
}


void setSeqNum(Datagram datagram, int seqNum)
{
	datagram->sequence = seqNum % MAXSEQNUM;
}
void setAckNum(Datagram datagram, int ackNum)
{
	datagram->ackNum = ackNum % MAXSEQNUM;
}

/*
    * Pack flags into Datagram.
    * Include seqNum and ACKNum that was last received from the intended recepient of the package.
    * 
    * Use NULL as input for receivedDatagram if no datagram has been received yet.
    * (NULL should only be used for SYN)
*/

void setHeader(Datagram datagram, int flag, int seqNum, int ackNum)
{
	datagram->windowSize = WINDOWSIZE;
	memset(datagram->message, 0, sizeof(datagram->message));
	datagram->flag = flag;
	datagram->checksum = 0;
	if(flag == SYN)
	{
		setSeqNum(datagram, MAXSEQNUM);
		setAckNum(datagram, 0);
		return;
	}
	setSeqNum(datagram, seqNum);
	setAckNum(datagram, ackNum);
}

int recvMessage(int sock, Datagram receivedMessage, struct sockaddr_in* receivedAdress)
{
	struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    unsigned int addrlen = sizeof(recvAddr);

    if (recvfrom(sock, (Datagram)receivedMessage, sizeof(Header),
        0, (struct sockaddr *)&recvAddr, &addrlen) < 0)
    {
		free(receivedMessage);
        perror("Error receiving message!\n");
		exit(EXIT_FAILURE);
    }
	if(corrupt(receivedMessage)) return ERRORCODE;
    *receivedAdress = recvAddr;
    return 1;
}

int sendMessage(int sock, Datagram messageToSend, struct sockaddr_in destAddr)
{
	if (sendto(sock, (Datagram)messageToSend, sizeof(Header),
	    0, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0)
    {
        perror("Failed to send message\n");
        return ERRORCODE;
    }
    return 1;
}

//!Abstract
void setDefaultHeader(Datagram messageToSend)
{
	messageToSend->windowSize = WINDOWSIZE;
	messageToSend->sequence = 1;
	messageToSend->flag = DATA;
	messageToSend->message[0] = '\0';
}

//!Abstract
Datagram initDatagram()
{
    Datagram temp = (Datagram)calloc(1 , sizeof(Header));
    if (temp == NULL)
    {
        perror("Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
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

//!Abstract
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
//!Abstract
void timeoutTest(int signum)
{
    printf("\nTimed out\n");

	if (signum == SIGKILL)
	{
		printf("\nExiting..");
		exit(EXIT_SUCCESS);
	}
} 

//!Abstract
void timeoutConnection(int sock, Datagram connRequest, struct sockaddr_in dest)
{
	connRequest->flag = SYN;

    if(sendMessage(sock, connRequest, dest) < 0)
    {
        perror("Could not send message to server");
        exit(EXIT_FAILURE);
    }
}

ConnectionInfo connectToServer(int sock, char* hostName)
{
    // Setup destination adress.
    struct hostent *hostInfo;
    hostInfo = gethostbyname(hostName);

	struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SERVERPORT);
    destAddr.sin_addr = *(struct in_addr *)hostInfo->h_addr;

	//? tempList is used as ConnectionInfo can't be initiated by itself (?)
	ClientList tempList = initClientList();
    // Attempt handshake with server
    if (initHandshakeWithServer(sock, destAddr, &tempList) != 1)
    {
        printf("Failed connection handshake.\n");
		// free(messageToSend);
        exit(EXIT_FAILURE);
    }
	// free(messageToSend);
    return tempList.clients[0];
}

//!Abstract
int initHandshakeWithServer(int sock, struct sockaddr_in dest, ClientList* list)
{
	// Setup first message to be sent.
    Datagram messageToSend = initDatagram();

    setHeader(messageToSend, SYN, 0, 0);
    strncpy(messageToSend->message, "LINE:257!\0", strlen("LINE:257!\0")); //! Test message, remove later
	messageToSend->checksum = calcChecksum(messageToSend, sizeof(*messageToSend));

	if(sendMessage(sock, messageToSend, dest) < 0)
	{
		perror("Could not send message to server.\n");
		exit(EXIT_FAILURE);
	}

    Datagram messageToReceive = initDatagram();
    struct sockaddr_in recvAddr;

	while(1)
	{
		//!TIMER
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
            setHeader(messageToSend, ACK, 0, messageToReceive->sequence);
			strncpy(messageToSend->message, "LINE:285!\0", strlen("LINE:285!\0")); //! Test message, remove later
			messageToSend->checksum = calcChecksum(messageToSend, sizeof(*messageToSend));

			if(sendMessage(sock, messageToSend, dest) == ERRORCODE)
			{
				printf("Could not send message to server\n");
				free(messageToReceive);
				return ERRORCODE;
			}
			if (addToClientList(list, initConnectionInfo(messageToReceive, recvAddr, sock)))
			{
				printf("Connection established\n");
				free(messageToReceive);
				return 1;
			}
			else
			{
				free(messageToReceive);
				return ERRORCODE;
			}
		}
	}
	free(messageToReceive);
	return ERRORCODE;
}
//?Split up into smaller more precise functions
int acceptClientConnection(int serverSock, ClientList* list)
{
	Datagram receivedDatagram = initDatagram();
	struct sockaddr_in recvAddr;
	if ((!recvMessage(serverSock, receivedDatagram, &recvAddr)) || isInClientList(list, recvAddr))
	{
		free(receivedDatagram);
		printf("\nLINE 316: Refused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
		return ERRORCODE;
	}
	// if (isInClientList(list, recvAddr))
	// {
	// 	free(receivedDatagram);
	// 	printf("\nRefused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
	// 	return ERRORCODE;
	// }

    struct sockaddr_in ACKaddr;
	Datagram toSend = initDatagram();

	/*	Socket is created here to avoid creating several sockets
		if the SYN+ACK gets lost.
	*/
	int clientSock = createClientSpecificSocket(recvAddr);
	while(1)
	{		
		if (receivedDatagram->flag == SYN)
		{
			printf("\nReceived SYN");
			setHeader(toSend, SYN + ACK, STARTSEQ, receivedDatagram->sequence);
			toSend->checksum = calcChecksum(toSend, sizeof(*toSend));
			

			// Send using the clientSock so client gets address of
			// designated port.
			if(sendMessage(clientSock, toSend, recvAddr) == ERRORCODE)
			{
				free(toSend);
				free(receivedDatagram);
				close(clientSock);
			    perror("Could not send message to client\n");
			    exit(EXIT_FAILURE);
			}
		}
        else	// First message from a new client must be SYN
		{
			free(toSend);
			free(receivedDatagram);
			close(clientSock);
			printf("\nLINE 361: Refused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
			return ERRORCODE;
		} 

        /*
            Wait for ACK from the expected address.
        ?   Might need to move this as the connection will be blocked if 
        ?   every ACK gets lost from this client.

            SYN could arrive here if the SYN+ACK got lost,
            if so then the server will resend SYN+ACK and then wait here again.
        */
        recvMessage(serverSock, receivedDatagram, &ACKaddr);
		
        //* Make sure that ACK is coming from expected adress
		//! Timeout need to trigger this state aswell
    	if(receivedDatagram->flag == ACK 
            && ACKaddr.sin_addr.s_addr == recvAddr.sin_addr.s_addr
            && ACKaddr.sin_port == recvAddr.sin_port)
        {
			if (addToClientList(list, initConnectionInfo(receivedDatagram, recvAddr, clientSock)))
			{
				printf("\nConnection established with client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
				free(receivedDatagram);
				free(toSend);
            	return clientSock;
			}
			free(toSend);
			free(receivedDatagram);
			close(clientSock);
            printf("\nRefused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
			return ERRORCODE;
        }
		recvAddr = ACKaddr;
	}
}

//!Abstract
void interpretPack_receiver(int sock, ClientList *clientList, fd_set* activeFdSet)
{
	Datagram receivedDatagram = initDatagram();
	ConnectionInfo *client = findClientFromSock(clientList, sock);
	printf("I got into interpret!\n");
	printf("clientSoc %d, sock %d\n",client->sock, sock);
	recvMessage(client->sock, receivedDatagram, &client->addr);

	//* Start disconnect process
	if (receivedDatagram->flag == FIN || (receivedDatagram->flag == ACK && isFINSet(*client))
		|| (isFINSet(*client) && client->FIN_SET_time.tv_sec > 2 * RTT)) 
	{
		DisconnectServerSide(client, receivedDatagram, clientList, activeFdSet);
	}
	else if (receivedDatagram->flag == ACK) return; // What is the client ACKing?

	//* Send to GBN or SR to handle DATA in package
	if (SWMETHOD == GBN) interpretWith_GBN_receiver(receivedDatagram, client, clientList);
	else interpretWith_SR_receiver(sock, receivedDatagram, client, clientList);
}

//!Abstract
void interpretWith_GBN_receiver(Datagram receivedDatagram, ConnectionInfo *client, ClientList *clientList)
{	
	/* Check if any timer has run out
	struct timespec currTime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &currTime);
	for (int i = 0; i < MAXSEQNUM; i++)
	{
		if (client->buffer[i].timeStamp.tv_sec - currTime.tv_sec > RTT)
		{
			//! Timer is not needed here, WTF you doin?
		}
	}*/

	if ((receivedDatagram->sequence == client->baseSeqNum) || (!corrupt(receivedDatagram)))
	{
		Datagram toSend = initDatagram();
		setHeader(toSend, ACK, 0, receivedDatagram->sequence);
		client->baseSeqNum++;
	}
	// else discard
}

//!Abstract
void interpretWith_SR_receiver(int sock, Datagram packet, ConnectionInfo *client, ClientList *clients)
{
	
    if(packet->sequence == client->baseSeqNum)
    {
        Datagram toSend = initDatagram();
        setHeader(toSend, ACK, 0, packet->sequence);
        client->baseSeqNum++;
    }
    
}




/* List functions */

//!Abstract
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

//!Abstract
ConnectionInfo initConnectionInfo(Datagram receivedDatagram, struct sockaddr_in recvAddr, int sock)
{
	ConnectionInfo tempInfo;
	if (receivedDatagram->flag == ACK) tempInfo.baseSeqNum = STARTSEQ;
	else if (receivedDatagram->flag == SYN + ACK) tempInfo.baseSeqNum = receivedDatagram->ackNum;
	else
	{
		perror("\nWrong usage of initConnectionInfo\n");
		exit(EXIT_FAILURE);
	}
	tempInfo.addr = recvAddr;
	tempInfo.FIN_SET = 0;
	tempInfo.sock = sock;
	for (int i = 0; i < MAXSEQNUM; i++) tempInfo.buffer[i].message[0] = '\0';
	for (int i = 0; i < MAXSEQNUM; i++)
	{
		tempInfo.buffer[i].timeStamp.tv_nsec = 0;
		tempInfo.buffer[i].timeStamp.tv_sec = 0;
	} 
	return tempInfo;
}

//!Abstract
int addToClientList(ClientList *list, ConnectionInfo info)
{
    int cur = list->size;
    list->size += 1;
    list->clients = realloc(list->clients, list->size * sizeof(ConnectionInfo));
    if (list->clients == NULL) return 0;
    list->clients[cur] = info;
    return 1;
}

//!Abstract
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

//!Abstract
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

//!Abstract
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

//!Abstract
ConnectionInfo* findClientFromSock(ClientList *list, int sock)
{
	if(list == NULL) return NULL;
	for (int i = 0; i < list->size; i++)
		if (list->clients[i].sock == sock) return &list->clients[i];
	return NULL;
}

//!Abstract
int setupClientDisconnect(int sock, char* hostName, struct sockaddr_in* destAddr)
{
    struct hostent* hostInfo;
    hostInfo = gethostbyname(hostName);
    destAddr->sin_family = AF_INET;
    destAddr->sin_port = htons(SERVERPORT);
    destAddr->sin_addr = *(struct in_addr *)hostInfo->h_addr;
    Datagram datagramToSend = initDatagram();

    if(DisconnectClientSide(sock, datagramToSend, *destAddr, 0) != 1)
    {
        perror("Failed disconnect handshake");
        exit(EXIT_FAILURE);

    }
    return datagramToSend->sequence;
}

int DisconnectClientSide(int sock, Datagram sendTo, struct sockaddr_in destAddr, int nextSeq)
{
    Datagram messageReceived = initDatagram();
    struct sockaddr_in tempAddr;
    int counter = 0;
	
    printf("In disconnect clientside\n");
	setHeader(sendTo, FIN, nextSeq, nextSeq);
	if(sendMessage(sock, sendTo, destAddr) < 0)
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
			setHeader(sendTo, ACK, 0, messageReceived->sequence);
			sendMessage(sock, sendTo, destAddr);
			 //Timer should be 2 * MSL

		}
		if(messageReceived->flag == FIN && counter > 0) //if counter is more than 0 that means it is the second FIN received
		{
			setHeader(sendTo, ACK, 0, messageReceived->sequence);
            printf("\nDisconnected\n");
			sendMessage(sock, sendTo, destAddr);
			 //restarts timer, if timer runs out client disconnects
            
			return 1; //temporary return
		}
		
		if(messageReceived->flag == ACK)
            counter++;
	}
}

/*
    * Functions to get values from ConnectionInfo
*/

//!Abstract
int getExpectedSeq(struct sockaddr_in addr, ClientList* list)
{
	if(list == NULL) return ERRORCODE;
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
            return list->clients[i].baseSeqNum;
    }
    // Return ERRORCODE as it can't find client.
    return ERRORCODE;
}

//!Abstract
int setBaseSeq(int seqToSet, struct sockaddr_in addr, ClientList* list)
{
	if(list == NULL) return ERRORCODE;
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
    return ERRORCODE;
}

//!Abstract
int setFIN(struct sockaddr_in addr, ClientList* list)
{
	if(list == NULL) return ERRORCODE;
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
    return ERRORCODE;
}

//!Abstract
int isFINSet(ConnectionInfo connection)
{
	if(connection.FIN_SET) return 1;
	return 0;
}

/*
    * Functions to set neccessary information in header depending on flag.
*/

int writeMessageSR(ConnectionInfo *server, char* message, int* currentSeq)
{
    int SRwindow = 0;
	Datagram toSend = initDatagram();
	packMessage(toSend, message, *currentSeq);

    while(1)
    {
        

        if(SRwindow < WINDOWSIZE)
        {
			strncpy(server->buffer[*currentSeq].message, message, sizeof(*message));
            if (sendMessage(server->sock, toSend, server->addr) < 0) return ERRORCODE;
            *currentSeq = (*currentSeq + 1) % MAXSEQNUM;      
            SRwindow++;
            //* Start TIMER
            printf("Implement later\n");
            return 1;
        }

		/* 
		! What is message->ackNum? Gives errors
        else if(SRwindow >= WINDOWSIZE && message->ackNum == server.baseSeqNum) 
        {
            server->buffer[message->ackNum].message = '\0';
            server->baseSeqNum++;
            SRwindow--;
        }
        */



        //recvMessage(server->sock, message, &server->addr);
    }


    
}



//! Change to use setHeader when brain starts working again
void packMessage(Datagram datagram, char* message, int currentSeq)
{
	setHeader(datagram, DATA, currentSeq, 0);
	if(message != NULL)
    	strncpy(datagram->message, message, strlen(message));
	datagram->checksum = calcChecksum(datagram, sizeof(*datagram));
}
