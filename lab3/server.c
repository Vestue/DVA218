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
#define MAXLENGTH 1024

int main() {
    int sock;
    if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0){
        perror("Could not create a socket\n");
		exit(EXIT_FAILURE);
    }
}