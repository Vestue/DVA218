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

#define SWMETHOD 0
/*
    Set if Go-Back-N or Selective Repeat should
    be used as the method for sliding windows.

    Go-Back-N = 0
    Selective Repeat = 1
*/

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

struct ConnectionInfo
{
    struct sockaddr_in addr;
    int expectedSeqNum;
    int FIN_SET;
};

/* Typedefs */

typedef struct Packet *Datagram;
typedef struct ClientList *ConnectionInfo;

/* Declared functions and descriptions */

int recvMessage(int sock, Datagram receivedMessage);
/*
    Attempt to read data from chosen socket.
    If there is no data to read function should return 0.
    
    Otherwise, the data should be added to sent Datagram
    and then return 1.
*/


int sendMessage(int sock, Datagram messageToSend, struct sockaddr_in destinationAddr);
/*
    Send Datagram to chosen socket by using the provided sockaddr.

    Return 1 if successful and 0 if not.
*/

int createSocket(int port);
/*
    Create socket using chosen port.
    Return created socket if successful.
    Otherwise print error and exit program.
*/

int connect(int sock, Datagram connectionReq);
/*
    Tries to connect to the server
    returns 1 if successfull



*/

int acceptConnection(int sock, Datagram connectionReq);
/*
    Accepts the connectionrequests
    returns 1 if succesfull


*/


void connectionTimeout(int sock, Datagram connectionReq);
/*
    Executes when a timeout has occured
    Not sure it works might get removed


*/


void setDefaultHeader(Datagram messageToSend);
/*
    Fill datagram with default information about
    window size, sequence number.
    Set flag to UNSET and set message to '\0'.
*/

void startTimer(int sock, Datagram timedConnection);
/*
    Set a timer for a certain sequence number.

Todo: The timer needs to be able to be connected to a certain 
Todo: package from a certain sockaddr.

Todo: It also needs to be able to call diffrent functions as handles
Todo: depending on the need upon time.
Todo: For example, resend a timed out package or close connection.

?   These parameters will have to be changed as i have no idea
?   what paramters need to be used.
*/


void stopTimer(Datagram timedConnection, int seqNum);
/*
    Use provided adress to stop timer for certain sequence number.
*/

void restartTimer(Datagram timedConnection, int seqNum);
/*
    Use provided adress to restart timer for certain sequence number.
*/

int getExpectedSeq(struct sockaddr_in addr, struct ConnectionInfo* clientList);
/*
    Return the expected sequence number from a certain sockaddr.

    Pointer is used to be able to get from either only one connection or
    multiple connections (if its sent as the typedeffed ClientList) 
*/

Datagram packACK(Datagram messageToSend, Datagram receivedMessage);
/*
    Make the message ready to be sent as an ACK using the sequence number
    within the received message.
    messageToSend should first get default values and then get the seq++
    and ACK flag.

*    Function returns the prepared datagram.
?    The use of this is to be able to do things like:
?        sendMessage(sock, packACK(messageToSend, receivedMessage), destinationAddr);
?   In one single step instead of needing to setDefaultMessage and then change flags
?   in the server or client itself.

*   Datagram types as paramters are used to increase abstraction for client and server.
*/

/*
    Make the message get default values and then set the SYN flag.
    Return altered datagram.
*/
Datagram packSYN(Datagram messageToSend);

//* Following packs are similar to the last one
//* as they will only restore the message to a default condition
//* and then set their flag.
//? Note that these functions are mostly used for abstraction.
Datagram packFIN(Datagram messageToSend);
Datagram packSYNACK(Datagram messageToSend);

#endif