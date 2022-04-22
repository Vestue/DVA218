/* File: client.c
 * Trying out socket communication between processes using the Internet protocol family.
 * Usage: client [host name], that is, if a server is running on 'lab1-6.idt.mdh.se'
 * then type 'client lab1-6.idt.mdh.se' and follow the on-screen instructions.
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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 5555
#define hostNameLength 50
#define messageLength  256

/* initSocketAddress
 * Initialises a sockaddr_in struct given a host name and a port.
 */
void initSocketAddress(struct sockaddr_in *name, char *hostName, unsigned short int port) {
    struct hostent *hostInfo; /* Contains info about the host */
    /* Socket address format set to AF_INET for Internet use. */
    name->sin_family = AF_INET;
    /* Set port number. The function htons converts from host byte order to network byte order.*/
    name->sin_port = htons(port);
    /* Get info about host. */
    hostInfo = gethostbyname(hostName);
    if(hostInfo == NULL) {
        fprintf(stderr, "initSocketAddress - Unknown host %s\n",hostName);
        exit(EXIT_FAILURE);
    }
    /* Fill in the host name into the sockaddr_in struct. */
    name->sin_addr = *(struct in_addr *)hostInfo->h_addr;
}
/* writeMessage
 * Writes the string message to the file (socket) 
 * denoted by fileDescriptor.
 */
void writeMessage(int fileDescriptor, char *message) {
    int nOfBytes = 0;
    //printf("%s", message);
    nOfBytes = write(fileDescriptor, message, strlen(message) + 1);
    if(nOfBytes < 0) {
        perror("writeMessage - Could not write data\n");
        exit(EXIT_FAILURE);
    }
}

int readMessageFromServer(int fileDescriptor) {
    char buffer[messageLength];
    int nOfBytes;

    nOfBytes = read(fileDescriptor, buffer, messageLength);
    if(nOfBytes < 0) {
        perror("Could not read data from server\n");
        exit(EXIT_FAILURE);
    }
    else if(nOfBytes == 0) {
        /* End of file */
        return(-1);
    }
    else {
        /* Data read */
        printf("\nFrom server: %s\n",  buffer);
    }
    return(0);
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serverName;
    char hostName[hostNameLength];
    char messageString[messageLength] = {'\0'};
  
    /* Check arguments */
    if(argv[1] == NULL) {
        perror("Usage: client [host name]\n");
        exit(EXIT_FAILURE);
    }
    else {
        strncpy(hostName, argv[1], hostNameLength);
        hostName[hostNameLength - 1] = '\0';
    }
    /* Create the socket */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Could not create a socket\n");
        exit(EXIT_FAILURE);
    }
    /* Initialize the socket address */
    initSocketAddress(&serverName, hostName, PORT);
    /* Connect to the server */
    if(connect(sock, (struct sockaddr *)&serverName, sizeof(serverName)) < 0) {
        perror("Could not connect to server\n");
        exit(EXIT_FAILURE);
    }
    /* Send data to the server */
    printf("\nType something and press [RETURN] to send it to the server.\n");
    printf("Type 'quit' to nuke this program.\n");
    fflush(stdin);

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);
    FD_SET(sock, &readSet);
    while (1) {
        fd_set testSet = readSet;

        if (select(sock + 1, &testSet, NULL, NULL, NULL) == -1)
        {
            printf("Select error");
            exit(EXIT_FAILURE);
        }

        // Go through the sockets and check if information
        // is coming from our listening socket or stdin.

        // We do this as we don't want to be able to get stuck in a fgets
        // as the client can't readMessages while fgets is waiting for '\n'.
        for (int i = 0; i < FD_SETSIZE; ++i)
        {
            if (i == STDIN_FILENO && FD_ISSET(i, &testSet))
            {
                fgets(messageString, messageLength, stdin);
                messageString[messageLength - 1] = '\0';
                if (strncmp(messageString, "quit\n", messageLength) != 0){
                    writeMessage(sock, messageString);
                }
                else {
                    close(sock);
                    exit(EXIT_SUCCESS);
                }
            }
            else if (i == sock && FD_ISSET(i, &testSet))
            {
                if(readMessageFromServer(sock) == -1) exit(EXIT_SUCCESS);
                printf("\n>");
            }
        }
        fflush(stdout);
    }
}