#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o stems.o clientGet.o clientPost.o alarmClient.o pushServer.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 
SQL = `mysql_config --cflags --libs`

.SUFFIXES: .c .o 

all: server clientPost clientGet dataGet.cgi dataPost.cgi alarmClient pushServer alarm.cgi

server: server.o request.o stems.o
	$(CC) $(CFLAGS) -o server server.o request.o stems.o $(LIBS)

clientGet: clientGet.o stems.o
	$(CC) $(CFLAGS) -o clientGet clientGet.o stems.o

clientPost: clientPost.o stems.o
	$(CC) $(CFLAGS) -o clientPost clientPost.o stems.o $(LIBS)

dataGet.cgi: dataGet.c stems.h /usr/include/mysql/mysql.h
	$(CC) $(CFLAGS) -o dataGet.cgi dataGet.c stems.o /usr/include/mysql/mysql.h $(SQL)

dataPost.cgi: dataPost.c stems.h /usr/include/mysql/mysql.h
	$(CC) $(CFLAGS) -o dataPost.cgi dataPost.c stems.o /usr/include/mysql/mysql.h $(SQL)

alarmClient: alarmClient.o stems.h
	$(CC) $(CFLAGS) -o alarmClient alarmClient.o stems.o $(LIBS)

pushServer: pushServer.o stems.h
	$(CC) $(CFLAGS) -o pushServer pushServer.o stems.o $(LIBS)

alarm.cgi: alarm.c stems.h
	$(CC) $(CFLAGS) -o alarm.cgi alarm.c
	
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

server.o: stems.h request.h
clientGet.o: stems.h
clientPost.o: stems.h
alarmClietn.o: stems.h
pushServer.o: stems.h

clean:
	-rm -f $(OBJS) server clientPost clientGet dataGet.cgi dataPost.cgi alarmClient alarm.cgi pushServer /fifo
