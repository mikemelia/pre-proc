CFLAGS=-Wall -g

all:	pre-proc

pre-proc: debug.h pre-proc.c

clean:
	rm -f pre-proc

