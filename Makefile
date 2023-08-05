
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all:
	$(CC) $(CFLAGS) make-regkey.c -o make-regkey -s
	$(CC) $(CFLAGS) check-regkey.c -o check-regkey -s
