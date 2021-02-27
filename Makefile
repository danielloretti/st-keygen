
CC = gcc
CFLAGS = -Wall -Wextra -pedantic

obj = make-regkey.o

make-regkey: make-regkey.o
	$(CC) make-regkey.o -o make-regkey -s
