# Define required macros here
SHELL = /bin/sh

EXES=disktest.x clearfilecache.x
OBJS=main.o
CFLAGS=-Wall -D_FILE_OFFSET_BITS=64
CC=g++
INCLUDES=
LIBS=-lpthread -lrt
all: ${EXES} 

disktest.x: main.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ main.o ${LIBS}

clearfilecache.x: clearfilecache.o
	${CC} ${CFLAGS} ${INCLUDES} -o $@ clearfilecache.o ${LIBS}

clean:
	-rm -f *.o core *.core *.gch

.cpp.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<
