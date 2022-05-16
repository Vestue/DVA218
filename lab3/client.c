/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "client.h"

int main(int argc, char *argv[])
{    
    int sock = createSocket(CLIENTPORT);
    struct sockaddr_in destAddr;
	
    if (argv[1] == NULL) 
    {
       perror("Usage: client [host name]\n");
       exit(EXIT_FAILURE);
    }
    char hostName[50];
    memset(&hostName, 0, sizeof(char));
    strncpy(hostName, argv[1], 50);
	hostName[50-1] = '\0';

    /*
        Begin connection attempt.
        Return value does not need to be checked as program will
        close upon error.
    */
    int currentSeq = setupServerConnection(sock, hostName, &destAddr);
    printf("%d\n", currentSeq);
    return 0;
}