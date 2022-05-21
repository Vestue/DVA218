# DVA218
Datakommunikation

## Planning
### MessageBuffer
- Should this buffer be windowsize?
  - NO, that causes issues with not being able to tell if a packet is old or new.

````c
struct messageBuffer[MAXSEQNUM]
{
	char* message;
	int timeStamp; //Kravspec, kanske ska vara på annat ställe
}
````

For serverside:
- if baseSeq = 30 and incSeq = 31, 32, 33
- They can be put into messageBuffer[31], messageBuffer[32], messageBuffer[33]

- then when incSeq = 30: Instantly unload messageBuffer[31], messageBuffer[32] and messagebuffer[33].
  - (Keep unloading until \0), unloaded messages should get \0 in the buffer.

Similar for clientSide:
- Unacked messages are put in the buffer and when, for example: ACK(63) arrives, it can set messageBuffer[63]
to be \0. Then when TIMER(63) triggers, it will read \0 and know not to send message.
- TIMER(63) would otherwise make client resend messageBuffer[63].

#### Points to note
- Something can not be put into the buffer if the distance between baseSeq and currSeq is bigger than windowSize.
- If an ACK gets lost the server will see that the incSeq is behind the baseSeq. And can then just ACK again.
- This buffer should be per **connection** so put it into the:
````c 
struct ConnectionInfo
````
