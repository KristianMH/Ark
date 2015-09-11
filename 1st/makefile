CC=gcc 
CFLAGS=-Werror -Wall -Wextra -pedantic -std=c11

SUBDIRS := $(wildcard */.)
all: $(SUBDIRS) 
$(SUBDIRS):
	 $(MAKE) -C $@

sim: mips32.h sim.c 
	$(CC) $(CFLAGS) -o sim sim.c

clean: $(SUBDIRSCLEAN)
	rm -f sim
$(SUBDIRSCLEAN):
	 $(MAKE) -C $@ clean


.PHONY: all $(SUBDIRS) clean
