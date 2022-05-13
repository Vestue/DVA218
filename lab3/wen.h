/****************************************************************
 *  - DVA218 - Lab 3    -
 *  - Group B3          -
 *  
 * - Fredrik Nygårds
 * - Oscar Einarsson
 * - Ragnar Winblad von Walter
 ****************************************************************/ 

#ifndef WEN_H
#define WEN_H

/* Defined macros */
#define MAXLENGTH 1024

// Receiver sets window size and maximum sequence number
#define WINDOWSIZE 64
#define MAXSEQNUM 128

/*
    Set if Go-Back-N or Selective Repeat should
    be used as the method for sliding windows.

    Go-Back-N = 0
    Selective Repeat = 1
*/
#define SWMETHOD 0

/* Enums */
enum slidingWindowMethods { GBN = 0, SR = 1 };

//! Enum itself can't be used as variable.
//! Remove comment when everyone has read.
enum flag { UNSET=0, SYN=1, ACK=2, SYNACK=3, FIN=4 };

/* Struct definitions */

struct Header 
{
    int windowSize;
    int sequence;
    int flag;
};

struct Packet 
{
    struct Header header;
    char* message;
};

/* Typedefs */

typedef struct Packet *Datagram;

/* Declared functions and descriptions */

/*
    Attempt to read data from chosen socket.
    If there is no data to read function should return 0.
    
    Otherwise, the data should be added to sent Datagram
    and then return 1.
*/
int recvMessage(int sock, Datagram receivedMessage);


/*
    Send Datagram to chosen socket by using the provided sockaddr.

    Return 1 if successful and 0 if not.
*/
int sendMessage(int sock, Datagram messageToSend, struct sockaddr_in destinationAddr);

/*
    Create socket using chosen port.
    Return created socket if successful.
    Otherwise print error and exit program.
*/
int createSocket(int port);

/*
    Fill datagram with default information about
    window size, sequence number.
    Should also get a flag for either GBN or SR.
*/
void setDefaultHeader(Datagram);

#endif