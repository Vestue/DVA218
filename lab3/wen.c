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


uint32_t calcChecksum(const void* M, uint32_t length)
{
    uint32_t P = 0x04C11DB7;
    const uint8_t* M8 = (const uint8_t*)M;
    uint32_t R = 0;
    for (uint32_t i = 0; i < length; ++i)
    {
        R ^= M8[i];
        for (uint32_t j = 0; j < 8; ++j)
        {
            R = R & 1 ? (R >> 1) ^ P : R >> 1;
        }
    }
    return R;
}

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
        return ERORRCODE;
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

ConnectionInfo connectToServer(int sock, char* hostName)
{
    // Setup destination adress.
    struct hostent *hostInfo;
    hostInfo = gethostbyname(hostName);

	struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(SERVERPORT);
    destAddr.sin_addr = *(struct in_addr *)hostInfo->h_addr;

    // Setup first message to be sent.
    Datagram messageToSend = initDatagram();
    setHeader(messageToSend, SYN, NULL);

    //! Test message, remove later
    char * msg = "Banana!\0";
    strncpy(messageToSend->message, msg, strlen(msg));

	//? tempList is used as ConnectionInfo can't be initiated by itself (?)
	ClientList tempList = initClientList();
    // Attempt handshake with server
    if (initHandshakeWithServer(sock, messageToSend, destAddr, &tempList) != 1)
    {
        printf("Failed connection handshake.\n");
		free(messageToSend);
        exit(EXIT_FAILURE);
    }
	free(messageToSend);
    return tempList.clients[0];
}

int initHandshakeWithServer(int sock, Datagram toSend, struct sockaddr_in dest, ClientList* list)
{
	if(sendMessage(sock, toSend, dest) < 0)
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
            setHeader(toSend, ACK, messageToReceive);
			if(sendMessage(sock, toSend, dest) == ERORRCODE)
			{
				perror("Could not send message to server\n");
				free(messageToReceive);
				return ERORRCODE;
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
				return ERORRCODE;
			}
		}
	}
	free(messageToReceive);
	return ERORRCODE;
}

int acceptClientConnection(int serverSock, ClientList* list)
{
	Datagram receivedDatagram = initDatagram();
	struct sockaddr_in recvAddr;
	if (recvMessage(serverSock, receivedDatagram, &recvAddr) != 1)
	{
		free(receivedDatagram);
		printf("\nRefused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
		return ERORRCODE;
	} 
	if (isInClientList(list, recvAddr))
	{
		free(receivedDatagram);
		printf("\nRefused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
		return ERORRCODE;
	}

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
			setHeader(toSend, SYN + ACK, receivedDatagram);
			

			// Send using the clientSock so client gets address of
			// designated port.
			if(sendMessage(clientSock, toSend, recvAddr) == ERORRCODE)
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
			printf("\nRefused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
			return ERORRCODE;
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
			return ERORRCODE;
        }
		recvAddr = ACKaddr;
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

int DisconnectServerSide(int sock, Datagram sendTo, struct sockaddr_in* dest)
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
        setHeader(sendTo, ACK, receivedDatagram);
        if(sendMessage(sock, sendTo, *dest) < 0)
        {
            printf("Failed to disconnect from client\n");
            exit(EXIT_FAILURE);
        }

        setHeader(sendTo, FIN, receivedDatagram);
    }
    

	while(1)
	{
        if (sendMessage(sock, sendTo, *dest) < 0)
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

int DisconnectClientSide(int sock, Datagram sendTo, struct sockaddr_in destAddr)
{
    Datagram messageReceived = initDatagram();
    struct sockaddr_in tempAddr;
    int counter = 0;
	
    printf("In disconnect clientside\n");
	setHeader(sendTo, FIN, messageReceived);
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
			setHeader(sendTo, ACK, messageReceived);
			sendMessage(sock, sendTo, destAddr);
			 //Timer should be 2 * MSL

		}
		if(messageReceived->flag == FIN && counter > 0) //if counter is more than 0 that means it is the second FIN received
		{
			setHeader(sendTo, ACK, messageReceived);
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
	//! Sequence numbers should be moved as they work diffrent depending on GBN/SR and sender/receiver
	//? SYN and SYN + ACK can still be used though
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

//! Change to use setHeader when brain starts working again
void packMessage(Datagram datagramToSend, char* messageToSend, int currentSeq)
{
	datagramToSend->windowSize = WINDOWSIZE;
	datagramToSend->sequence = currentSeq;
    strncpy(datagramToSend->message, messageToSend, strlen(messageToSend));
}