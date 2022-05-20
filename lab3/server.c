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
	int serverSock = createSocket(SERVERPORT);
	int clientSock;
    ClientList clients = initClientList();

	fd_set activeFdSet, readFdSet;
	FD_ZERO(&activeFdSet);
	FD_ZERO(&readFdSet);
	FD_SET(serverSock, &activeFdSet);

	while (1) 
	{
		// Copy active set to read-set for select on read-set
		readFdSet = activeFdSet;
		if (select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0)
		{
			perror("\nFailed to monitor set");
			//* FD_ZERO prevents reusing old set if select gets interrupted by timer
			FD_ZERO(&readFdSet);
			//exit(EXIT_FAILURE);
		}

		for (int currSock = 0; currSock < FD_SETSIZE; currSock++)
		{
			// * New connection incoming
			if (currSock == serverSock && FD_ISSET(currSock, &readFdSet))
			{
				clientSock = acceptClientConnection(serverSock, &clients);
				if (clientSock != ERORRCODE) FD_SET(clientSock, &activeFdSet);
			}
			// * Receiving from connected client
			else if (FD_ISSET(currSock, &readFdSet))
			{
				printf("\nI'm working on it!\n");
				printf("Reading from socket %d\n", currSock);
				//interpretPack_receiver(serverSock, receivedMessage, receivedAdress, &clients);
			}
		}

		/* Move this
		if (sequenceNumber >= MAXSEQNUM)
			sequenceNumber = 0;
		*/
	}
	return 0;
}

void closeConnection(ClientList *list, struct sockaddr_in addr)
{
    //Disconnect client
	//DisconnectServerSide(serverSock, receivedMessage, &receivedAdress);
    removeFromClientList(list, addr);
}