
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

make-regkey: make-regkey.o
	$(CC) make-regkey.o -o make-regkey -s
