#ifndef WEN_H
#define WEN_H



struct Header {
    int windowSize;
    int sequence;
    enum flag{GBN=0, SER=1, SYN=2, ACK=3, FIN=4, SYNACK=5};
};

struct Packet {
    struct Header header;
    char* message;
};

typedef Packet *Datagram;

#endif