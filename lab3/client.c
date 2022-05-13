/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "wen.c"
#include "client.h"

#define MAXLENGTH 512
#define PORT 5555

int main()
{
    int sock = createSocket();
	int sequenceNumber = 0;
	Datagram receivedMessage;
	Datagram messageToSend;
	setDefaultHeader(messageToSend);
    sockaddr_in destAddr;

    if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0)
    {
        perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
    }

    return 0;
}

int connectToServer(int sock, Datagram connectionReq)
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

        signal(SIGALRM, timeoutConnection(sock, connectionReq));
        alarm(2);
        if(connectionReq->header.flag = SYNACK)
        {      
            connectionReq->header.flag = ACK;
            if(sendMessageToServer(sock, connectionReq) < 0)
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

void timeoutConnection(int sock, Datagram connectionReq)

connectionReq->header.flag = SYN;
{
    if(sendMessageToServer(sock, connectionReq) < 0)
    {
        perror(Could not send message to server);
        exit(EXIT_FAILURE);
    }
    

}

int sendMessageToServer(int sock, Datagram toSend, sockaddr_in destAddr)
{

	sendto(sock, (Datagram)&toSend, sizeof(toSend), 0, )
    return 1;
}
int recvMessageFromServer(int socket, Datagram receivedMessage)
{
    return 1;
}
void setDefaultHeader(Datagram messageToSend)
{
	messageToSend->header.windowSize = WINDOWSIZE;
	messageToSend->header.sequence = 1;
	messageToSend->header.flag = GBN;
	messageToSend->message = '\0';
}