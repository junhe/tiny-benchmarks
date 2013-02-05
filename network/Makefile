# Define required macros here
SHELL = /bin/sh

EXES=server.x client.x
CFLAGS=-Wall -D_FILE_OFFSET_BITS=64
CC=g++
INCLUDES=
LIBS=-lpthread -lrt
all: ${EXES} 

server.x: server.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ server.o ${LIBS}

client.x: client.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ client.o ${LIBS}

clean:
	-rm -f *.o core *.core *.gch

.cpp.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<
