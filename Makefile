CC = gcc
CFLAGS = -Wall -g -lncurses

all: tetrix

tetrix: main.o tetrix.o
	$(CC) -o tetrix main.o tetrix.o $(CFLAGS)

main.o: main.c tetrix.h
	$(CC) -c main.c $(CFLAGS)

tetrix.o: tetrix.c tetrix.h
	$(CC) -c tetrix.c $(CFLAGS)

clean:
	rm -f tetrix main.o tetrix.o
