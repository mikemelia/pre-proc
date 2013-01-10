CFLAGS=-Wall -g

all:	proof

proof: debug.h proof.c

clean:
	rm -f proof

