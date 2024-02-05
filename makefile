CC = g++
CFLAGS = -std=c++2a -Wall -Wextra -pedantic -g

.DEFAULT_GOAL := hsh

hsh: main.o shell.o
	$(CC) -o hsh main.o shell.o

main.o: main.cpp shell.h
	$(CC) $(CFLAGS) -c main.cpp 

shell.o: shell.cpp shell.h
	$(CC) $(CFLAGS) -c shell.cpp

clean :
	rm -f hsh depend.mak *.o
	rm -r -f *.dSYM