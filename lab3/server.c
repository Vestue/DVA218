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
    struct sockaddr_in receivedAdress;

	acceptConnection(sock, receivedMessage, &receivedAdress);

    ClientList clients = initClientList();
    printf("Just before loop\n");
    // TODO need to add dynamic array functions before this loop will work
	while (1) 
	{
		if (recvMessage(sock, receivedMessage, &receivedAdress)) 
		{
            printf("\nServer: Connect from client %s, port %d\n", inet_ntoa(receivedAdress.sin_addr), ntohs(receivedAdress.sin_port));
        }

        if (isInClientList(&clients, receivedAdress))
        {
            if (acceptConnectionInLoop(sock, receivedMessage, &receivedAdress) == 0)
            {
                printf("Refused connection from client %s, port %d\n", inet_ntoa(receivedAdress.sin_addr), ntohs(receivedAdress.sin_port));
            }
        } 
        else interpretPack(sock, receivedMessage, receivedAdress, &clients);

		if (sequenceNumber >= MAXSEQNUM)
			sequenceNumber = 0;
	}
	return 0;
}

int acceptConnection(int sock, Datagram connRequest, struct sockaddr_in* dest)
{
	printf("Before connection loop");
    struct sockaddr_in tempAddr;
	while(1)
	{
		recvMessage(sock, connRequest, &tempAddr);
		
		if (connRequest->header.flag == SYN)
		{
            *dest = tempAddr;
			printf("Received SYN\n");
			connRequest->header.flag = SYN + ACK;
			signal(SIGALRM, timeoutTest);
			alarm(2);
			if(sendMessage(sock, connRequest, *dest) == 0)
			{
			    perror("Could not send message to client");
			    exit(EXIT_FAILURE);
			}
		}
		
        //* Make sure that ACK is coming from expected adress
    	if(connRequest->header.flag == ACK 
            && tempAddr.sin_addr.s_addr == dest->sin_addr.s_addr
            && tempAddr.sin_port == dest->sin_port)
        {
            printf("Connection established");
            return 1;
        }
	}
}

//! This one is currently WIP
int acceptConnectionInLoop(int sock, Datagram connRequest, struct sockaddr_in* dest)
{
	printf("Before connection loop");
    struct sockaddr_in tempAddr;
	while(1)
	{		
		if (connRequest->header.flag == SYN)
		{
            *dest = tempAddr;
			printf("Received SYN\n");
			connRequest->header.flag = SYN + ACK;
			signal(SIGALRM, timeoutTest);
			alarm(2);
			if(sendMessage(sock, connRequest, *dest) == 0)
			{
			    perror("Could not send message to client");
			    exit(EXIT_FAILURE);
			}
		}
        else return 0; // First message from a new client must be SYN

        /*
            Wait for ACK from the expected address.
        ?   Might need to move this as the connection will be blocked if 
        ?   every ACK gets lost from this client.

            SYN could arrive here if the SYN+ACK got lost,
            if so then the server will resend SYN+ACK and then wait here again.
        */
        recvMessage(sock, connRequest, &tempAddr);
		
        //* Make sure that ACK is coming from expected adress
    	if(connRequest->header.flag == ACK 
            && tempAddr.sin_addr.s_addr == dest->sin_addr.s_addr
            && tempAddr.sin_port == dest->sin_port)
        {
            printf("Connection established");
            return 1;
        }
	}
}

void interpretPack(int sock, Datagram packet, struct sockaddr_in addr, ClientList *clients)
{
    if (packet->header.flag == ACK && findClient(clients, addr)->FIN_SET == 1)
    {
        closeConnection(clients, addr);
    }

	if (SWMETHOD == GBN) interpretWithGBN(sock, packet, addr, clients);
	else interpretWithSR(sock, packet, addr, clients);
}

void interpretWithGBN(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients)
{
	Datagram messageToSend = initDatagram();
    printf("%d", messageToSend->header.windowSize); //? Just to get rid of warnings
}

void interpretWithSR(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients)
{
	Datagram messageToSend = initDatagram();
    printf("%d", messageToSend->header.windowSize); //? Just to get rid of warnings
}

void closeConnection(ClientList *list, struct sockaddr_in addr)
{
    //Disconnect client
    removeFromClientList(list, addr);
}

/* List functions */

ClientList initClientList()
{
    ClientList list;
    struct ConnectionInfo* tempArr = (struct ConnectionInfo*)calloc(1, sizeof(struct ConnectionInfo));
    if (tempArr == NULL)
    {
        perror("Failed to allocate memory for client list");
        exit(EXIT_FAILURE);
    }
    list.clients = tempArr;
    list.size = 0;
    return list;
}

int addToClientList(ClientList *list, struct ConnectionInfo info)
{
    int cur = list->size;
    list->size += 1;
    list->clients = realloc(list->clients, list->size * sizeof(struct ConnectionInfo));
    if (list->clients == NULL) return 0;
    list->clients[cur] = info;
    return 1;
}

int removeFromClientList(ClientList *list, struct sockaddr_in addr)
{
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
            struct ConnectionInfo* tempArr = (struct ConnectionInfo*)calloc(list->size, sizeof(struct ConnectionInfo));
            if (tempArr == NULL)
            {
                perror("Failed to allocate memory for client list");
                exit(EXIT_FAILURE);
            }
            // Copy everything before the index
            if (i != 0) memcpy(tempArr, list->clients, i * sizeof(struct ConnectionInfo));
            // Copy after index
            if (i != list->size - 1) 
                memcpy(tempArr + i, list->clients + i + 1, (list->size - i - 1) * sizeof(struct ConnectionInfo));
            free(list->clients);
            list->clients = tempArr;
            return 1;
        }
    }
    return 0;
}

int isInClientList(ClientList *list, struct sockaddr_in addr)
{
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
            return 1;
    }
    return 0;
}

struct ConnectionInfo* findClient(ClientList *list, struct sockaddr_in addr)
{
    struct sockaddr_in tempAddr;
    for (int i = 0; i < list->size; i++)
    {
        tempAddr = list->clients[i].addr;
        if(tempAddr.sin_addr.s_addr == addr.sin_addr.s_addr && tempAddr.sin_port == addr.sin_port)
            return &list->clients[i];
    }
    return NULL;
}