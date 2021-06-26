NAME          := libfstring
VERSION_MAJOR := 1
VERSION_MINOR := 0
VERSION       := $(VERSION_MAJOR).$(VERSION_MINOR)

FNAME   := $(NAME)
SNAME   := $(FNAME).a
DNAME   := $(FNAME).so.$(VERSION)

CC=gcc
CFLAGS=-Wall -g -O0 
LDFLAGS=-shared -soname=$(FNAME).so.$(VERSION_MAJOR)
TEST_CFLAGS=-O0 -Wall -g
all: build

build:
	$(CC) $(CFLAGS) -fPIC -c fstring.c -o fstring.o
	$(LD) $(LDFLAGS) -o $(DNAME) fstring.o
	$(AR) $(ARFLAGS) $(SNAME) fstring.o
	@ldconfig -v -n .
	$(CC) $(TEST_CFLAGS) test.c fstring.c -o test

clean:
	rm -f fstring test $(SNAME) $(DNAME) *.o
