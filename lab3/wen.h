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
#define MAXLENGTH 512
#define SERVERPORT 5555
// Set clientport to 0 so OS assigns any avaible port
#define CLIENTPORT 5556

// Receiver sets window size and maximum sequence number
#define WINDOWSIZE 64
#define MAXSEQNUM 128
#define ERORRCODE -1
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

typedef struct
{
	uint16_t windowSize;
	uint32_t sequence;
    uint32_t ackNum;
	uint8_t flag;
	uint_16_t checksum;
    char message[MAXLENGTH];
}Header;

// struct Packet
// {
// 	struct Header header;
// 	char message[MAXLENGTH];
// };

typedef struct
{
	struct sockaddr_in addr;
	int baseSeqNum;
	int FIN_SET;
}ConnectionInfo;



/* Typedefs */

typedef Header *Datagram;

typedef struct
{
	ConnectionInfo *clients;
	size_t size;
} ClientList;

/* Declared functions and descriptions */

/*
    Allocate memory for a datagram.
    Initialise content to default values if successfull
    and return the datagram.
*/
Datagram initDatagram();

/*
	Attempt to read data from chosen socket.
	If there is no data to read function should return 0.
	
	Otherwise, the data should be added to sent Datagram
	and then return 1.
*/
int recvMessage(int sock, Datagram receivedMessage, struct sockaddr_in* receivedAdress);

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
    Setup information required for connection, go through the connection handshake
    with the server.
    Return sequence number sent in the server SYN+ACK.
*/
int setupServerConnection(int sock, char* hostName, struct sockaddr_in* destAddr);

/*
	Accepts the connectionrequests
	Return socket to connected client
	Return -1 if failed
*/
int acceptConnection(int sock, Datagram connRequest, struct sockaddr_in* dest);


//  !Everything below should be abstracted out
/*
    Tries to connect to the server
    returns 1 if successfull
*/
int connectToServer(int sock, Datagram connRequest, struct sockaddr_in dest);

/**
 *
 *  Not sure it works might get removed
 *	Executes when a timeout has occured
 */
void timeoutConnection(int sock, Datagram connRequest, struct sockaddr_in dest);



int DisconnectServerSide(int sock, Datagram disconnRequest, struct sockaddr_in* dest);


int DisconnectClientSide(int sock, Datagram disconnRequest, struct sockaddr_in dest);


/*
	Set a timer for a certain sequence number.

Todo: The timer needs to be able to be connected to a certain 
Todo: package from a certain sockaddr.

Todo: It also needs to be able to call diffrent functions as handles
Todo: depending on the need upon time.
Todo: For example, resend a timed out package or close connection.

?   These parameters will have to be changed as i have no idea
?   what parameters need to be used.
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

//!  Feeling cute might delete later :3
void timeoutTest();
/*
	Return the expected sequence number from a certain sockaddr.

	Pointer is used to be able to get from either only one connection or
	multiple connections (if its sent as the typedeffed ClientList) 
*/
int getExpectedSeq(struct sockaddr_in addr, ConnectionInfo* connections);

/*
	Set base sequense number of chosen connection to 
	sequence number sent as argument.
*/
void setBaseSeq(int seqToSet, struct sockaddr_in addr, ConnectionInfo *connections);

/*
	Set that FIN has been received from connection.
	FIN_SET = 1
*/
void setFIN(struct sockaddr_in addr, ConnectionInfo *connections);

/*
	Read FIN_SET in chosen connection.
	Return 1 if the FIN state is set,
	0 if it isn't.
*/
int FINisSet(struct sockaddr_in addr, ConnectionInfo *connections);


/*
    Todo: Make more generic, example: setHeader(Datagram messageToSend, uint8_t flag)

	Fill datagram with default information about
	window size, sequence number.
	Set flag to UNSET and set message to '\0'.
*/
void setDefaultHeader(Datagram messageToSend);

/*
    * Pack flags into Datagram.
    * Include seqNum and ACKNum that was last received from the intended recepient of the package.
    * Use NULL as input if there are no known numbers.
    * (NULL should only be used for SYN)
*/
void setHeader(Datagram datagramToSend, int flag, int lastReceivedSeq, int lastReceivedACK_Num);

/*
    Pack message into datagram and set correct information for a data packet.
*/
void packMessage(Datagram datagramToSend, char* messageToSend, int lastReceivedSeq, int lastReceivedACK_Num);

void interpretPack_receiver(int sock, Datagram packet, struct sockaddr_in addr, ClientList *clients);

/*
*   GO BACK N Functions
*/

/*
	Main function of GBN, this takes the data and then does different
	things depending on what flag is in the datagram.
*/
void interpretWith_GBN_receiver(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients);
//         switch (receivedMessage->header.flag)
            //{
            //	case SYN:
            //		// Send flag SYN+ACK
            //		messageToSend->header.flag = SYN + ACK;
            //                 //sendMessageToClient(sock, messageToSend);
            //		// TODO Start timer
            //                 break;
            //	//! Can recieve ACKs, FINs, or data
            //	//! before connection even has been attempted
            //	//? Check if client_addr is in client addr
            //             case ACK:
            //		// TODO Chill timer
            //		break;
            //	case FIN:
            //		/*kill*/
            //		break;
            //	default:
            //		/*Destroy and ACK old packages*/
            //		/*Normal packet*/
            //		messageToSend->header.flag = ACK;
            //		messageToSend->header.sequence = receivedMessage->header.sequence + 1;
            //		break;
            //}
            // TODO: if SYN_timer_timeouts

/*
*   Selective repeat functions
*/

void interpretWith_SR_receiver(int sock, Datagram packet, struct sockaddr_in destAddr, ClientList *clients);

/*
	Allocate memory to a list used for client connection info.
	(malloc)

	Return pointer to client list if successful, print error if not.
*/
ClientList initClientList();

/*
    Begin by checking if client is in list.
	Reallocate memory to make room for new client and then
	add client info to the dynamic array.

	Return 1 if successful, 0 if not.
*/
int addToClientList(ClientList *list, ConnectionInfo info);

/*
    Attempt to remove client from list.
    First check if client is in list, then remove and reallocate.
    
    Return 1 if successful, 0 if not.
*/
int removeFromClientList(ClientList *list, struct sockaddr_in addr);

/*
	Check if the sockaddr exists in the list.

	Return 1 if it does, 0 if it doesn't.
*/
int isInClientList(ClientList *list, struct sockaddr_in addr);

/*
    Search the list for the client and add the client.
    Return NULL if it can't be found. 
*/
ConnectionInfo* findClient(ClientList *list, struct sockaddr_in addr);

#endif