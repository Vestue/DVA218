/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

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
int SRwindow = 0;

//TODO: Check all functions references and remove unused functions
//TODO: Check all prints to see if there is any debug code left
//TODO: Give every function a comment on how it works
//TODO: Change all currentSeq to nextSeq
//TODO: Change name of all datagrams
//TODO: Doublecheck all function names

/*
	Calculates checksum for datagram
	returns the result of calculation
	
	Usage: datagram->checksum = calcChecksum(datagram, sizeof(*datagram))
*/

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

int corrupt(Datagram datagram)
{
	//Creates a copy of datagram with a zero'd checksum
	Datagram zeroedChecksum = (Datagram)calloc(1, sizeof(*datagram));
	memcpy(zeroedChecksum, datagram, sizeof(*datagram));
	zeroedChecksum->checksum = 0;
	zeroedChecksum->checksum = calcChecksum((Datagram)zeroedChecksum, (sizeof(*zeroedChecksum)));

	//? Add error inducing code here later
	// example: if rand<25 return 1
	if (datagram->checksum == zeroedChecksum->checksum)
		return 0;
	return 1;
}

/*
	!DON'T FORGET TO CALCULATE
	!CHECKNUM AFTER SETTING MESSAGE
	* Set up header with given parameters
	* - DATA: seqNum = nextSeq, ackNum = nextSeq+1
	* - SYN: seqNum = 0, ackNum = 0
	* - ACK: seqNum = 0, ackNum = recv.seq
	* - SYNACK: seqNum = startSeq, ackNum = recv.seq
	* - FIN: seqNum = nextSeq, ackNum = nextSeq+1
*/
void setHeader(Datagram datagram, int flag, int seqNum, int ackNum)
{
	datagram->windowSize = WINDOWSIZE;
	memset(datagram->message, 0, sizeof(datagram->message));
	datagram->flag = flag;
	datagram->checksum = 0;
	if(flag == SYN)
	{
		datagram->seqNum = MAXSEQNUM;
		datagram->ackNum = 0;
		return;
	}
	datagram->seqNum = seqNum;
	datagram->ackNum = ackNum;
}

/*
    Pack message into datagram and set correct information for a data packet.
*/
void packMessage(Datagram datagram, char* message, int nextSeq, int ackNum, int flag)
{
	setHeader(datagram, flag, nextSeq, ackNum);

	if(message != NULL)
    	strncpy(datagram->message, message, strlen(message));

	else memset(datagram->message, 0, sizeof(datagram->message));
	datagram->checksum = calcChecksum(datagram, sizeof(*datagram));
}

int recvMessage(int sock, Datagram datagram, struct sockaddr_in* receivedAdress)
{
	struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    unsigned int addrlen = sizeof(recvAddr);

    if (recvfrom(sock, (Datagram)datagram, sizeof(Header),
        0, (struct sockaddr *)&recvAddr, &addrlen) < 0)
    {
		free(datagram);
        perror("Error receiving message!\n");
		exit(EXIT_FAILURE);
    }
	if(corrupt(datagram)) return ERRORCODE;
    *receivedAdress = recvAddr;
    return 1;
}

int sendMessage(int sock, Datagram datagram, struct sockaddr_in destAddr)
{
	if (sendto(sock, (Datagram)datagram, sizeof(Header),
	    0, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0)
    {
        perror("Failed to send message\n");
        return ERRORCODE;
    }
    return 1;
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

void timeoutExit(int signum)
{
	if (signum == SIGALRM)
	{
		printf("\nTimeout reached\nDisconnected!\n\n");
		exit(EXIT_SUCCESS);
	}
} 

void timeoutServerConnection(int signal)
{
	printf("Timed out.\n");
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

//TODO: fix this bonkers shit
/*
    Tries to connect to the server.
    Returns 1 if successful, ERRORCODE if not.
	Put server information into the ConnectionInfo in the ClientList upon connection.
*/
int initHandshakeWithServer(int sock, struct sockaddr_in dest, ClientList* list)
{
	// Setup first message to be sent.

    Datagram connRequest = initDatagram();
	packMessage(connRequest, NULL, 0, 0, SYN);
    // setHeader(connRequest, SYN, 0, 0);
	// connRequest->checksum = calcChecksum(connRequest, sizeof(*connRequest));

	printf("Sending SYN..\n");
	if(sendMessage(sock, connRequest, dest) < 0)
	{
		perror("Could not send message to server.\n");
		exit(EXIT_FAILURE);
	}
	struct timespec time_SYN_sent, time_current;
	clock_gettime(CLOCK_MONOTONIC_RAW ,&time_SYN_sent);
	time_current.tv_sec = 0;
	free(connRequest);
    Datagram messageToReceive = initDatagram();
    struct sockaddr_in recvAddr;
	fd_set activeFdSet, readFdSet;
	FD_ZERO(&activeFdSet);
	FD_ZERO(&readFdSet);
	FD_SET(sock, &activeFdSet);
	struct timeval selectTime;
	selectTime.tv_sec = 2 * RTT;
	selectTime.tv_usec = 0;

	while(1)
	{
		readFdSet = activeFdSet;
		// Resend SYN upon timeout
		clock_gettime(CLOCK_MONOTONIC_RAW, &time_current);
		if (time_current.tv_sec - time_SYN_sent.tv_sec >= 2 * RTT)
		{
			printf("Sending SYN..\n");
			if(sendMessage(sock, connRequest, dest) < 0)
			{
				perror("Could not send message to server.\n");
				exit(EXIT_FAILURE);
			}
			clock_gettime(CLOCK_MONOTONIC_RAW, &time_SYN_sent);
		}

		if ((select(FD_SETSIZE, &readFdSet, NULL, NULL, &selectTime) < 0))
		{
			//* FD_ZERO prevents reusing old set if select gets interrupted by timer
			
			FD_ZERO(&readFdSet);
		}
		if (FD_ISSET(sock, &readFdSet))
		{
			if (recvMessage(sock, messageToReceive, &recvAddr) == 0)
			{
				free(messageToReceive);
				return ERRORCODE;
			}
		}
        

		if(messageToReceive->flag == SYN + ACK)
		{
			printf("Received SYN+ACK\n");

			//TODO: Check if function is used correctly
			Datagram datagramACK = initDatagram();
			packMessage(datagramACK, NULL, datagramACK->seqNum, datagramACK->seqNum, ACK);
            // setHeader(datagramACK, ACK, 0, messageToReceive->seqNum);
			// datagramACK->checksum = calcChecksum(datagramACK, sizeof(*datagramACK));
			// sleep(100);
			printf("Responding with ACK\n");

			if(sendMessage(sock, datagramACK, dest) == ERRORCODE)
			{
				printf("Could not send message to server\n");

				free(messageToReceive);

				return ERRORCODE;
			}
			
			if (addToClientList(list, initConnectionInfo(messageToReceive, recvAddr, sock)))
			{
				printf("\nConnection established!\n\n");
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


//TODO: Fix so it uses states
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

    struct sockaddr_in ACKaddr;
	Datagram toSend = initDatagram();
	struct timespec time_SYNACK_sent, time_current;
	time_SYNACK_sent.tv_sec = 0;
	time_current.tv_sec = 0;

	/*	Socket is created here to avoid creating several sockets
		if the SYN+ACK gets lost.
	*/
	int clientSock = createClientSpecificSocket(recvAddr);
	fd_set activeFdSet, readFdSet;
	FD_ZERO(&activeFdSet);
	FD_ZERO(&readFdSet);
	FD_SET(serverSock, &activeFdSet);
	struct timeval selectTime;
	selectTime.tv_sec = 2 * RTT;
	selectTime.tv_usec = 0;
	while(1)
	{		
		readFdSet = activeFdSet;
		if (receivedDatagram->flag == SYN)
		{
			printf("Received SYN\n");
			//TODO: Check if function is used correctly
			packMessage(toSend, NULL, STARTSEQ, receivedDatagram->seqNum, SYN+ACK);
			// setHeader(toSend, SYN + ACK, STARTSEQ, receivedDatagram->seqNum);
			// toSend->checksum = calcChecksum(toSend, sizeof(*toSend));
			printf("Sending SYN+ACK..\n");

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
			clock_gettime(CLOCK_MONOTONIC_RAW, &time_SYNACK_sent);
		}
        else	// First message from a new client must be SYN
		{
			free(toSend);
			free(receivedDatagram);
			close(clientSock);
			printf("\nLINE 358: Refused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
			return ERRORCODE;
		} 

        /*
            Wait for ACK from the expected address.
        ?   Might need to move this as the connection will be blocked if 
        ?   every ACK gets lost from this client.

            SYN could arrive here if the SYN+ACK got lost,
            if so then the server will resend SYN+ACK and then wait here again.
        */
		
	   	if ((select(FD_SETSIZE, &readFdSet, NULL, NULL, &selectTime) < 0))
		{
			//* FD_ZERO prevents reusing old set if select gets interrupted by timer
			
			FD_ZERO(&readFdSet);
		}
		if (FD_ISSET(serverSock, &readFdSet))
        	recvMessage(serverSock, receivedDatagram, &ACKaddr);

		clock_gettime(CLOCK_MONOTONIC_RAW, &time_current);
        //* Make sure that ACK is coming from expected adress
    	if((receivedDatagram->flag == ACK 
            && ACKaddr.sin_addr.s_addr == recvAddr.sin_addr.s_addr
            && ACKaddr.sin_port == recvAddr.sin_port)
			|| (time_current.tv_sec - time_SYNACK_sent.tv_sec >= 2 * RTT))
        {
			if (addToClientList(list, initConnectionInfo(receivedDatagram, recvAddr, clientSock)))
			{
				printf("\nConnection established with client %s, port %d\n\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
				free(receivedDatagram);
				free(toSend);
            	return clientSock;
			}
			free(toSend);
			free(receivedDatagram);
			close(clientSock);
            printf("\nLINE 389: Refused connection from client %s, port %d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
			return ERRORCODE;
        }
		recvAddr = ACKaddr;
	}
}

ConnectionInfo* findClientFromSock(ClientList *list, int sock)
{
	if(list == NULL) return NULL;
	for (int i = 0; i < list->size; i++)
		if (list->clients[i].sock == sock) return &list->clients[i];
	return NULL;
}

int isFINSet(ConnectionInfo connection)
{
	if(connection.FIN_SET) return 1;
	return 0;
}

/*
	Copies and empties buffer string for printing
*/
void emptyBuffer(ConnectionInfo *client, Datagram datagram, char* message)
{
	if (message == NULL) printf("I am fucked\n");
	for (int i = client->baseSeqNum; client->buffer[i].message[0] != '\0'; i = ((i+1) % MAXSEQNUM))
	{
		strncat(message, client->buffer[i].message, strlen(client->buffer[i].message));
		memset(client->buffer[i].message, 0, strlen(client->buffer[i].message));
		client->baseSeqNum = (client->baseSeqNum + 1) % MAXSEQNUM;
	}
}

void interpretWith_SR_receiver(int sock, Datagram datagram, ConnectionInfo *client, ClientList *clients)
{
	//Empties buffer when you get base sequence number
	if(datagram->seqNum == client->baseSeqNum)
	{
		strncpy(client->buffer[datagram->seqNum].message, datagram->message, strlen(datagram->message));
		char message[MESSAGELENGTH*WINDOWSIZE+1];
		emptyBuffer(client, datagram, message);
		printf("Received message: \n%s\n", message);
	}
	//Copies message to buffer for later printing
	else
	{
		strncpy(client->buffer[datagram->seqNum].message, datagram->message, strlen(datagram->message));
	}
	time_t currTime;
	time(&currTime);
	Datagram datagramACK = initDatagram();
	packMessage(datagramACK, NULL, datagramACK->seqNum, datagramACK->seqNum, ACK);
	// setHeader(datagramACK, ACK, 0, datagramACK->seqNum);
	// datagramACK->checksum = calcChecksum(datagramACK, sizeof(*datagramACK));
	sendMessage(sock, datagramACK, client->addr);
	printf("Responding with ACK(%d)\n%s", datagramACK->seqNum, ctime(&currTime));
}

/*
	Main function of GBN, this takes the data and then does different
	things depending on what flag is in the datagram.
*/
void interpretWith_GBN_receiver(Datagram receivedDatagram, ConnectionInfo *client, ClientList *clientList)
{	
	printf("seq: %d | base: %d\n", receivedDatagram->seqNum, client->baseSeqNum);
	if ((receivedDatagram->seqNum == client->baseSeqNum))
	{
		printf("Received message: \n%s\n", receivedDatagram->message);
		Datagram datagramACK = initDatagram();
		packMessage(datagramACK, NULL, datagramACK->seqNum, datagramACK->seqNum, ACK);
		// setHeader(datagramACK, ACK, 0, receivedDatagram->seqNum);
		// datagramACK->checksum = calcChecksum(datagramACK, sizeof(*datagramACK));
		if (sendMessage(client->sock, datagramACK, client->addr))
		{	
			time_t currTime;
			time(&currTime);
			printf("Responding with ACK(%d)\n%s", client->baseSeqNum, ctime(&currTime));

			client->baseSeqNum = (client-> baseSeqNum+1)%MAXSEQNUM;
		}
		else 
			printf("Failed to send ACK(%d)!\n", client->baseSeqNum);
	}
}

void interpretPack_receiver(int sock, ClientList *clientList, fd_set* activeFdSet)
{
	Datagram datagram = initDatagram();
	ConnectionInfo *client = findClientFromSock(clientList, sock);

	int retval;
	retval = recvMessage(client->sock, datagram, &client->addr);
	if (retval == 0)
	{
		printf("No data to read.\n");
		return;
	}
	else if (retval == ERRORCODE)
	{
		printf("Package corrupted!\n");
		return;
	}
	printf("Receiving data..\n");
	//* Start disconnect process
	if (datagram->flag == FIN || (datagram->flag == ACK && isFINSet(*client))
		|| (isFINSet(*client) && client->FIN_SET_time.tv_sec > 2 * RTT)) 
	{
		DisconnectServerSide(client, datagram, clientList, activeFdSet);
	}
	else if (datagram->flag == ACK) return; // What is the client ACKing?

	//* Send to GBN or SR to handle DATA in package
	else if (SWMETHOD == GBN) interpretWith_GBN_receiver(datagram, client, clientList);
	else interpretWith_SR_receiver(sock, datagram, client, clientList);
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

//!Abstract
ConnectionInfo initConnectionInfo(Datagram connRequest, struct sockaddr_in recvAddr, int sock)
{
	ConnectionInfo tempInfo;
	memset(&tempInfo, 0, sizeof(tempInfo));
	if (connRequest->flag == ACK || connRequest->flag == SYN) tempInfo.baseSeqNum = STARTSEQ;
	else if (connRequest->flag == SYN + ACK) tempInfo.baseSeqNum = connRequest->ackNum;
	else
	{
		perror("\nWrong usage of initConnectionInfo\n");
		exit(EXIT_FAILURE);
	}
	tempInfo.addr = recvAddr;
	tempInfo.FIN_SET = 0;
	tempInfo.FIN_SET_time.tv_nsec = 0;
	tempInfo.FIN_SET_time.tv_sec = 0;
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


//!Abstract


//!Abstract


int DisconnectServerSide(ConnectionInfo* client, Datagram discRequest, ClientList* clientList, fd_set* activeFdSet)
{
	struct timespec currTime;
	clock_gettime(CLOCK_MONOTONIC_RAW, &currTime);
	time_t finTime;

	if ((discRequest->flag == FIN)) 
	{
		printf("Received FIN!\n");
		Datagram datagramFIN = initDatagram();
		packMessage(datagramFIN, NULL, discRequest->ackNum, discRequest->seqNum, FIN);
    	// setHeader(datagramFIN, FIN, discRequest->ackNum, discRequest->seqNum);
		// datagramFIN->checksum = calcChecksum(datagramFIN, sizeof(*datagramFIN));
		if (sendMessage(client->sock, datagramFIN, client->addr) < 0)
		{
			printf("Failed to disconnect client\n");
			return ERRORCODE;
		}
		printf("Responding with FIN\n");
		clock_gettime(CLOCK_MONOTONIC_RAW, &client->FIN_SET_time);
	}

	// Fully disconnect client by removing and closing socket.
	// This timeout is used if all ACKs gets lost from the client
	else if ((discRequest->flag == ACK && isFINSet(*client))
		|| (isFINSet(*client) && (currTime.tv_sec - client->FIN_SET_time.tv_sec) > 4 * RTT))
	{
		time(&finTime);
        printf("\nDisconnected client %s, port %d\n", inet_ntoa(client->addr.sin_addr), ntohs(client->addr.sin_port));
		printf("%s", ctime(&finTime));
		close(client->sock);
		FD_CLR(client->sock, activeFdSet);
		removeFromClientList(clientList, client->addr);
	}

	//TODO: see if neccessary
	// Resend FIN
	//? Needs to be last if-state as this checks for a shorter elapsed time
	//? than previous one
	else if (isFINSet(*client) && (currTime.tv_sec - client->FIN_SET_time.tv_sec) > 2 * RTT)
	{
		Datagram datagramFIN = initDatagram();
		packMessage(datagramFIN, NULL, discRequest->ackNum, discRequest->seqNum, FIN);
		// setHeader(datagramFIN, FIN, discRequest->ackNum, discRequest->seqNum);
		// datagramFIN->checksum = calcChecksum(datagramFIN, sizeof(*datagramFIN));
		if (sendMessage(client->sock, datagramFIN, client->addr) < 0)
		{
			printf("Failed to disconnect client\n");
			return ERRORCODE;
		}
	}

	return 1;
}

int DisconnectClientSide(ConnectionInfo server, int nextSeq)
{
	Datagram discRequest = initDatagram();
    Datagram datagramInc = initDatagram();
    struct sockaddr_in tempAddr;
	
	//TODO: Check if function is used correctly
	packMessage(discRequest, NULL, nextSeq, nextSeq, FIN);
	// setHeader(discRequest, FIN, nextSeq, nextSeq);
	// discRequest->checksum = calcChecksum(discRequest, sizeof(*discRequest));
	struct timespec time_current, time_FIN_sent;
	time_FIN_sent.tv_sec = 0;
	time_t finTime;

	while(1)
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &time_current);
		if (time_current.tv_sec - time_FIN_sent.tv_sec > 2 * RTT)
		{
			if(sendMessage(server.sock, discRequest, server.addr) < 0)
			{
				printf("Failed to disconnect from server\n");
				exit(EXIT_FAILURE);
			}
			time(&finTime);
			printf("Sending FIN..\n %s", ctime(&finTime));
			clock_gettime(CLOCK_MONOTONIC_RAW, &time_FIN_sent);
		}
		if (recvMessage(server.sock, datagramInc, &tempAddr) == ERRORCODE)
			printf("Received corrupt packet!\n"); 
		if (datagramInc->flag == ACK)
		{
			printf("Received ACK\n");
			break;
		}
		else if (datagramInc->flag == FIN) break;
	}
	signal(SIGALRM, timeoutExit);
	while(1)
	{
		if(datagramInc->flag == FIN)
		{
			printf("Disconnecting...\n");
			packMessage(discRequest, NULL, 0, datagramInc->seqNum, ACK);
			// setHeader(discRequest, ACK, 0, datagramInc->seqNum);
			// discRequest->checksum = calcChecksum(discRequest, sizeof(*discRequest));
			sendMessage(server.sock, discRequest, server.addr);
			alarm(2 * RTT);
		}
		recvMessage(server.sock, datagramInc, &tempAddr);
	}
}

/*
    * Functions to get values from ConnectionInfo
*/


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



//? Move this when testing is done
int writeMessage(ConnectionInfo *server, char* message, int *nextSeq)
{
	Datagram datagram = initDatagram();
	packMessage(datagram, message, *nextSeq, *nextSeq, DATA);
	time_t currTime;
	time(&currTime);

	for(int i = server->baseSeqNum, count = 0; i != *nextSeq; i = (i+1) % MAXSEQNUM, count++)
		if (count >= WINDOWSIZE) return 0;

	printf("Message sent at: %s", ctime(&currTime));
	printf("-with SEQ(%d)\n", datagram->seqNum);

	if (sendMessage(server->sock, datagram, server->addr) < 0) return ERRORCODE;
	
	strncpy(server->buffer[*nextSeq].message, message, strlen(message));
	clock_gettime(CLOCK_MONOTONIC_RAW, &server->buffer[*nextSeq].timeStamp);
	return 1;
}