/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#include "server.h"
#include "wen.h"
#include <errno.h>

int main() 
{
	int serverSock = createSocket(SERVERPORT);
	int clientSock;
    ClientList clients = initClientList();

	fd_set activeFdSet, readFdSet;
	FD_ZERO(&activeFdSet);
	FD_ZERO(&readFdSet);
	FD_SET(serverSock, &activeFdSet);

	// Timers and clocks to test things
	struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);

	// Print startup time for demonstration
	time_t beforeLoopTime;
	time(&beforeLoopTime);
	printf("\nStarting server: %s", ctime(&beforeLoopTime));
	while (1) 
	{
		// Copy active set to read-set for select on read-set
		readFdSet = activeFdSet;
		if (select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0)
		{
			perror("\nFailed to monitor set");
			printf("Errno is: %d\n", errno);
			//* FD_ZERO prevents reusing old set if select gets interrupted by timer
			FD_ZERO(&readFdSet);
			//exit(EXIT_FAILURE);

			// Test clock
			clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
			printf("Stopped at: %lu\n", stop.tv_sec);
			printf("\nElapsed time before timeout: %lu sec\n", stop.tv_sec - start.tv_sec);
		}

		for (int currSock = 0; currSock < FD_SETSIZE; currSock++)
		{
			// * New connection incoming
			if (currSock == serverSock && FD_ISSET(currSock, &readFdSet))
			{
				// Timers for testing purpose
				clock_gettime(CLOCK_MONOTONIC_RAW, &stop);
				printf("\nConnection request after: %lu seconds\n", stop.tv_sec - start.tv_sec);

				clientSock = acceptClientConnection(serverSock, &clients);
				if (clientSock != ERRORCODE) FD_SET(clientSock, &activeFdSet);
			}
			// * Receiving from connected client
			else if (FD_ISSET(currSock, &readFdSet))
			{
				printf("------------------------\n");
				printf("Reading from socket %d\n", currSock);
				interpretPack_receiver(currSock, &clients, &activeFdSet);
				printf("------------------------\n");
				// sleep(2);
			}
		}
	}
	return 0;
}