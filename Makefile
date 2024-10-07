# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# define targets
TARGETS = run_sfl

build: $(TARGETS)

run_sfl: sfl.o list.o block.o
	$(CC) $(CFLAGS) -o sfl sfl.o list.o block.o -lm

sfl.o: sfl.c list.h block.h
	$(CC) $(CFLAGS) -c -o sfl.o sfl.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c -o list.o list.c

block.o: block.c block.h list.h
	$(CC) $(CFLAGS) -c -o block.o block.c

clean:
	-rm -f sfl sfl.o list.o block.o
	-rm -f *~

