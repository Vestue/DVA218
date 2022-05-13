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
#include <netinet/in.h>
#include <netdb.h>
#include "client.h"

#define MAXLENGTH 512
#define PORT 5555

int main()
{
    int sock;
    if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0){
        perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
    }

    return 0;
}

sendMessageToServer(int sock)
{

    return 1;
}

recvMessageFromServer
{

    return 1;
}