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
typedef enum 
{ 
	UNSET=0,
	SYN=1,
	ACK=2,
	SYNACK=3,
	FIN=4,
}flag;


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
	int baseSeqNum;
	int FIN_SET;
};


/* Typedefs */

typedef struct Packet *Datagram;
typedef struct ConnectionInfo *ClientList;


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
	Tries to connect to the server
	returns 1 if successfull
*/
int connect(int sock, Datagram connectionReq, struct sockaddr_in dest);

/*
	Accepts the connectionrequests
	returns 1 if succesfull
*/
int acceptConnection(int sock, Datagram connectionReq, struct sockaddr_in dest);

/*
	Executes when a timeout has occured
	Not sure it works might get removed
*/
void connectionTimeout(int sock, Datagram connectionReq, struct sockaddr_in dest);




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
void startTimer(int sock, Datagram timedConnection);

/*
	Use provided adress to stop timer for certain sequence number.
*/
void stopTimer(Datagram timedConnection, int seqNum);

/*
	Use provided adress to restart timer for certain sequence number.
*/
void restartTimer(Datagram timedConnection, int seqNum);




/*
	Return the expected sequence number from a certain sockaddr.

	Pointer is used to be able to get from either only one connection or
	multiple connections (if its sent as the typedeffed ClientList) 
*/
int getExpectedSeq(struct sockaddr_in addr, struct ConnectionInfo* connections);

/*
	Set base sequense number of chosen connection to 
	sequence number sent as argument.
*/
void setBaseSeq(int seqToSet, struct sockaddr_in addr, struct ConnectionInfo *connections);

/*
	Set that FIN has been received from connection.
	FIN_SET = 1
*/
void setFIN(struct sockaddr_in addr, struct ConnectionInfo *connections);

/*
	Read FIN_SET in chosen connection.
	Return 1 if the FIN state is set,
	0 if it isn't.
*/
int FINisSet(struct sockaddr_in addr, struct ConnectionInfo *connections);




/*
	Fill datagram with default information about
	window size, sequence number.
	Set flag to UNSET and set message to '\0'.
*/
void setDefaultHeader(Datagram messageToSend);

/*
	Make the message ready to be sent as an ACK using the sequence number.
	messageToSend should first get default values and then get the seq++
	and ACK flag.

*    Function returns the prepared datagram.
?    The use of this is to be able to do things like:
?        sendMessage(sock, packACK(messageToSend, nextSeqNum), destinationAddr);
?   In one single step instead of needing to setDefaultMessage and then change flags
?   in the server or client itself.

*   Datagram types as paramters are used to increase abstraction for client and server.
*/
Datagram packACK(Datagram messageToSend, int nextSeqNum);

/*
	Make the message get default values and then set the SYN flag.
	Returns altered datagram.
*/
Datagram packSYN(Datagram messageToSend);

/*
	Make the message get default values and then set the SYN+ACK flag.
	Then set a starting point for sequence number to be used.
	Returns altered datagram.
*/
Datagram packSYNACK(Datagram messageToSend, int startingSeq);

/*
	Make the message get default values and then set the FIN flag.
	Returns altered datagram.
*/
Datagram packFIN(Datagram messageToSend);

/*
	Make message get default values and then add a string and
	the next sequence number to it.
	Returns altered datagram.
*/
Datagram packData(Datagram messageToSend, char *dataToPack, int nextSeqNum);

/*
*   GO BACK N Functions
*/

/*
	Main function of GBN, this takes the data and then does different
	things depending on what flag is in the datagram.
*/
void GBN_interpretData(Datagram receivedMessage, struct sockaddr_in receivedFromAddr, struct ConnectionInfo *connections);

/*
*   Selective repeat functions
*/

#endif