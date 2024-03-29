/* File: server.c
 * Trying out socket communication between processes using the Internet protocol family.
 */

/****************************************************************
 *  - DVA218 - Lab 2    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>

#define PORT 5555
#define MAXMSG 512
#define EMPTY -1

void writeMessage(int fileDescriptor, char* message);
int blackListed (char* IpAddr);

/* makeSocket
 * Creates and names a socket in the Internet
 * name-space. The socket created exists
 * on the machine from which the function is 
 * called. Instead of finding and using the
 * machine's Internet address, the function
 * specifies INADDR_ANY as the host address;
 * the system replaces that with the machine's
 * actual address.
 */
int makeSocket(unsigned short int port) {
	int sock;
	struct sockaddr_in name;
	
	/* Create a socket. */
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
	}
	/* Give the socket a name. */
	/* Socket address format set to AF_INET for Internet use. */
	name.sin_family = AF_INET;
	/* Set port number. The function htons converts from host byte order to network byte order.*/
	name.sin_port = htons(port);
	/* Set the Internet address of the host the function is called from. */
	/* The function htonl converts INADDR_ANY from host byte order to network byte order. */
	/* (htonl does the same thing as htons but the former converts a long integer whereas
	* htons converts a short.) 
	*/
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Assign an address to the socket by calling bind. */
	if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
		perror("Could not bind a name to the socket\n");
		exit(EXIT_FAILURE);
	}
	return(sock);
}

/* readMessageFromClient
 * Reads and prints data read from the file (socket
 * denoted by the file descriptor 'fileDescriptor'.
 */
int readMessageFromClient(int fileDescriptor) {
	char buffer[MAXMSG];
	int nOfBytes;

	nOfBytes = read(fileDescriptor, buffer, MAXMSG);
	if(nOfBytes < 0) {
		perror("Could not read data from client\n");
		exit(EXIT_FAILURE);
	}
	/* End of file */
	else if(nOfBytes == 0) return(-1);

	else {
		/* Data read */
		printf(">Incoming message: %s\n",  buffer);

		// Reply to the client.
		// The same fileDescriptor is used as the socket can
		// be used for both writing and reading.
		char* message = "Received";
		writeMessage(fileDescriptor, message);
	}
	return(0);
}

void writeMessage(int fileDescriptor, char *message) {
	int nOfBytes;
	
	nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
	if(nOfBytes < 0) {
		perror("writeMessage - Could not write data\n");
		exit(EXIT_FAILURE);
	}
}

void broadcast(fd_set activeFdSet, int serverSock){
	char* broadcastMessage = "A new client has connected!";
	fd_set writeFdSet = activeFdSet;
	for (int i = 0; i < FD_SETSIZE; i++)
	{
		if (FD_ISSET(i, &activeFdSet) && (i != serverSock))
		{
			if (select(FD_SETSIZE, NULL, &writeFdSet, NULL, NULL) < 0) {
				perror("Select failed\n");
				exit(EXIT_FAILURE);
			}
			writeMessage(i, broadcastMessage);
		}
	}
}

int main(int argc, char *argv[]) {
	int sock;
	int clientSocket;
	int i;
	char* welcomeMessage = "HELLO!";
	fd_set activeFdSet, readFdSet; /* Used by select */
	struct sockaddr_in clientName;
	socklen_t size;

	char* connectionRefusedMessage = "Connection refused.";
  
	/* Create a socket and set it up to accept connections */
	sock = makeSocket(PORT);
	/* Listen for connection requests from clients */
	if(listen(sock,1) < 0) {
		perror("Could not listen for connections\n");
		exit(EXIT_FAILURE);
	}
	/* Initialize the set of active sockets */
	FD_ZERO(&activeFdSet);
	FD_SET(sock, &activeFdSet);

	printf("\n[waiting for connections...]\n");
	while(1) {
		/* Block until input arrives on one or more active sockets
		FD_SETSIZE is a constant with value = 1024 */
		readFdSet = activeFdSet;
		if(select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0) {
			perror("Select failed\n");
			exit(EXIT_FAILURE);
		}
		/* Service all the sockets with input pending */
		for(i = 0; i < FD_SETSIZE; ++i)
		{
			if(FD_ISSET(i, &readFdSet)) {
				if(i == sock) {
					/* Connection request on original socket */
					size = sizeof(struct sockaddr_in);
					/* Accept the connection request from a client. */
					
					clientSocket = accept(sock, (struct sockaddr *)&clientName, (socklen_t *)&size); 
					
					// Check if the ip-address is a blacklisted address.
					if(clientSocket < 0) {
						perror("Could not accept connection\n");
						exit(EXIT_FAILURE);
					}

					if (blackListed(inet_ntoa(clientName.sin_addr))) {
						writeMessage(clientSocket, connectionRefusedMessage);
						printf("Server: Refused connection from client %s, port %d\n", inet_ntoa(clientName.sin_addr), ntohs(clientName.sin_port));
						close(clientSocket);
						

					}
					else {
						writeMessage(clientSocket, welcomeMessage);
						printf("Server: Connect from client %s, port %d\n", inet_ntoa(clientName.sin_addr), ntohs(clientName.sin_port));
					
						// Sends a message to all connected clients in the FdSet.
						// This is done before setting the new client to only broadcast to other clients.
						broadcast(activeFdSet, sock);
						FD_SET(clientSocket, &activeFdSet);
					}
				}
				else {
					/* Data arriving on an already connected socket */
					if (readMessageFromClient(i) < 0) {
						close(i);
						FD_CLR(i, &activeFdSet);
					}
				}
			}
		}
	}
}

int blackListed (char* IpAddr){
	char list[4][50] = { "127.0.03", "192.150.28.1", "128.0.0.2", "127.0.0.3"};

	for (int i = 0; i < 4; i++){
		if (strcmp(IpAddr, list[i]) == 0) return 1;
	}
	return 0;
}
