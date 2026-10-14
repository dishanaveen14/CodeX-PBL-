CC=gcc
CFLAGS=-Wall -Wextra -std=c11
SRC=$(wildcard *.c)

scheduler: $(SRC) common.h
	$(CC) $(CFLAGS) -o scheduler $(SRC)

clean:
	rm -f scheduler *.o
