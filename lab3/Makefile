CC = gcc
CFLAGS = -Wall -o
PROGRAMS = client server
DEPENDENCIES = wen.h wen.c

all: ${PROGRAMS}

client: client.c client.h $(DEPENDENCIES)
	${CC} ${CFLAGS} client client.c client.h $(DEPENDECIES)

server: server.c server.h $(DEPENDENCIES)
	${CC} ${CFLAGS} server server.c server.h $(DEPENDECIES)

.PHONY: clean

clean:
	rm -f ${PROGRAMS}
