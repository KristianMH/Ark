CC=gcc 
CFLAGS=-Werror -Wall -Wextra -pedantic -std=c11
all: sim

elf.o: elf.h elf.c
	gcc -c elf.c

sim: mips32.h elf.o sim.c 
	echo "Compiling simulator"
	$(MAKE) -C asm
	$(MAKE) -C c 
	$(CC) $(CFLAGS) -g -o sim elf.o sim.c

clean:
	$(MAKE) -C asm clean
	$(MAKE) -C c clean
	rm -f sim
	rm -f *.o
	echo "Folder is cleaned"

.PHONY: all clean 
