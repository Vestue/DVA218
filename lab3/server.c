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
	int sock = createSocket(SERVERPORT);
	int sequenceNumber = 0;
	Datagram receivedMessage = initDatagram();
    struct sockaddr_in receivedAdress;

	TEMPacceptConnection(sock, receivedMessage, &receivedAdress);

    ClientList clients = initClientList();
    printf("Just before loop\n");
    // TODO need to add dynamic array functions before this loop will work
	while (1) 
	{
		if (recvMessage(sock, receivedMessage, &receivedAdress)) 
		{
            printf("\nServer: Connect from client %s, port %d\n", inet_ntoa(receivedAdress.sin_addr), ntohs(receivedAdress.sin_port));
        }

        if (isInClientList(&clients, receivedAdress) == 0)
        {
            if (acceptConnection(sock, receivedMessage, &receivedAdress) == 0)
            {
                printf("Refused connection from client %s, port %d\n", inet_ntoa(receivedAdress.sin_addr), ntohs(receivedAdress.sin_port));
            }
        } 
        else if (receivedMessage->header.flag == ACK && findClient(&clients, receivedAdress)->FIN_SET == 1)
            closeConnection(&clients, receivedAdress);
        else interpretPack_receiver(sock, receivedMessage, receivedAdress, &clients);

		if (sequenceNumber >= MAXSEQNUM)
			sequenceNumber = 0;
	}
	return 0;
}

// ! Remove when start to use main loop in server
int TEMPacceptConnection(int sock, Datagram connRequest, struct sockaddr_in* dest)
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

void closeConnection(ClientList *list, struct sockaddr_in addr)
{
    //Disconnect client
    removeFromClientList(list, addr);
}