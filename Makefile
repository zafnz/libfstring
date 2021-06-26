CC=gcc

all: build

build:
	$(CC) -O0 -Wall -g -o fstring fstring.c

clean:
	rm fstring
