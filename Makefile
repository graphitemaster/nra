CC=gcc
CFLAGS=-Wall -Wextra -O2
OBJS=main.o

all: nra

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

nra: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(OBJS) nra

.PHONY: clean
