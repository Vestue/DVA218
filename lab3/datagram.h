#ifndef DATAGRAM_H
#define DATAGRAM_H

struct Header {
    int windowSize;
    int sequence;
    int ack;
    int flag;
};

struct Datagram {
    struct Header header;
    char* message;
};

#endif