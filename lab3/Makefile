CC = gcc
CFLAGS = -Wall -o
PROGRAMS = client server
DEPENDENCIES = wen.h wen.c 

all: ${PROGRAMS}

client: client.c $(DEPENDENCIES)
	${CC} ${CFLAGS} client client.c $(DEPENDENCIES)

server: server.c server.h $(DEPENDENCIES)
	${CC} ${CFLAGS} server server.c $(DEPENDENCIES)

.PHONY: clean

clean:
	rm -f ${PROGRAMS}
