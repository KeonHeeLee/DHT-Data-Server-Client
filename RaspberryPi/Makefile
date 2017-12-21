#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = clientRPI.o stems.o
CC = gcc
CFLAGS = -g -Wall
LIBS = -lpthread 

.SUFFIXES: .c .o 
all: clientRPI

clientRPI: clientRPI.o stems.o
	$(CC) $(CFLAGS) -o clientRPI clientRPI.o stems.o $(LIBS) -lwiringPi

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clientRPI.o: stems.h

clean:
	-rm -f $(OBJS) clientRPI stems.o

