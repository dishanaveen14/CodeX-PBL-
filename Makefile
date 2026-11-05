CC=gcc
CFLAGS=-Wall -Wextra -std=c11
LDLIBS=-lgdi32 -lcomctl32
SRC=$(wildcard *.c)

scheduler: $(SRC) common.h gui.h
	$(CC) $(CFLAGS) -o scheduler $(SRC) $(LDLIBS)

clean:
	rm -f scheduler *.o
