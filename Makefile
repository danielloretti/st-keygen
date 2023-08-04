
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all: clean make-regkey check-regkey

make-regkey: make-regkey.o
	$(CC) make-regkey.o -o make-regkey -s

check-regkey: check-regkey.o
	$(CC) check-regkey.o -o check-regkey -s

clean:
	rm -f *.o
