CC = gcc
CFLAGS = -Wall -o
PROGRAMS = client server
DEPENDENCIES = wen.h wen.c

all: ${PROGRAMS}

client: client.c $(DEPENDENCIES)
	${CC} ${CFLAGS} client client.c $(DEPENDENCIES) client.h

server: server.c server.h $(DEPENDENCIES)
	${CC} ${CFLAGS} server server.c $(DEPENDENCIES) server.h

.PHONY: clean

clean:
	rm -f ${PROGRAMS}
