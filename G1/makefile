.PHONY: all clean
CC=gcc
CFLAGS=-Werror -Wall -Wextra -pedantic -std=c11
sim: mips32.h sim.c
	$(CC) $(CFLAGS) -o sim sim.c
all: sim
clean:
	rm -f sim
