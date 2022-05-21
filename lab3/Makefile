CC = gcc
CFLAGS = -Wall -o
PROGRAMS = client server
DEPENDENCIES = wen.c wen.h

all: ${PROGRAMS}

client: client.c client.h $(DEPENDENCIES)
	${CC} ${CFLAGS} client client.c

server: server.c server.h $(DEPENDENCIES)
	${CC} ${CFLAGS} server server.c

.PHONY: clean

clean:
	rm -f ${PROGRAMS}
