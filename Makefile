CC = gcc
CFLAGS = -Wall -g -lncurses


all: clean tetrix client

tetrix: main.o tetrix.o
	$(CC) -o tetrix main.o tetrix.o $(CFLAGS)

main.o: main.c tetrix.h
	$(CC) -c main.c $(CFLAGS)

tetrix.o: tetrix.c tetrix.h
	$(CC) -c tetrix.c $(CFLAGS)

client: client.c tetrix.o
	$(CC) -o client client.c tetrix.o $(CFLAGS)

clean:
	rm -f tetrix main.o tetrix.o client
