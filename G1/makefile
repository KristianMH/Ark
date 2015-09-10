CC=gcc 
CFLAGS=-Werror -Wall -Wextra -pedantic -std=c11

SUBDIRS := $(wildcard */.)
all: $(SUBDIRS) 
$(SUBDIRS):
	 $(MAKE) -C $@

sim: mips32.h sim.c 
	$(CC) $(CFLAGS) -o sim sim.c
clean: 
	rm -f sim 

.PHONY: all $(SUBDIRS) clean
