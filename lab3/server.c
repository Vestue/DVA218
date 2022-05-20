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

    ClientList clients = initClientList();
	while (1) 
	{
		if (recvMessage(sock, receivedMessage, &receivedAdress)) 
		{
            printf("\nServer: Connect from client %s, port %d\n", inet_ntoa(receivedAdress.sin_addr), ntohs(receivedAdress.sin_port));
        }

        if (isInClientList(&clients, receivedAdress) == 0)
        {
            if (acceptClientConnection(sock, receivedMessage, &receivedAdress, &clients) == 0)
            {
                printf("\nRefused connection from client %s, port %d\n", inet_ntoa(receivedAdress.sin_addr), ntohs(receivedAdress.sin_port));
            }
			else
			{
				//! Move this when disconnect can be called via message
				DisconnectServerSide(sock, receivedMessage, &receivedAdress);
				closeConnection(&clients, receivedAdress);
			} 
        } 
        else if (receivedMessage->flag == ACK && findClient(&clients, receivedAdress)->FIN_SET == 1)
            closeConnection(&clients, receivedAdress);
        else interpretPack_receiver(sock, receivedMessage, receivedAdress, &clients);

		if (sequenceNumber >= MAXSEQNUM)
			sequenceNumber = 0;
	}
	return 0;
}

void closeConnection(ClientList *list, struct sockaddr_in addr)
{
    //Disconnect client
    removeFromClientList(list, addr);
}