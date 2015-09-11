CC=gcc 
CFLAGS=-Werror -Wall -Wextra -pedantic -std=c11
all: sim

sim: mips32.h sim.c 
	echo "Compiling simulator"
	$(MAKE) -C asm
	$(MAKE) -C c 
	$(CC) $(CFLAGS) -o sim sim.c

clean:
	$(MAKE) -C asm clean
	$(MAKE) -C c clean
	rm -f sim
	echo "Folder is cleaned"

.PHONY: all clean 
