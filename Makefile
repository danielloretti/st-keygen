CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all:
	$(CC) $(CFLAGS) make-regkey.c -o make-regkey
	$(CC) $(CFLAGS) check-regkey.c -o check-regkey

