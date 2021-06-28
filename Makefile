# fstring Makefile 
# 
# Copyright Nick Clifford, 2021
#
# Nick Clifford (nick@crypto.geek.nz)
#
#
NAME          := libfstring
VERSION_MAJOR := 1
VERSION_MINOR := 0
VERSION       := $(VERSION_MAJOR).$(VERSION_MINOR)

FNAME   := $(NAME)
SNAME   := $(FNAME).a
DNAME   := $(FNAME).so.$(VERSION)

CC=gcc
CFLAGS=-Wall -g  
LDFLAGS=-shared -soname=$(FNAME).so.$(VERSION_MAJOR)
TEST_CFLAGS=-O0 -Wall -g
AR=ar

all: build

build:
	@echo "Compiling fstring library"
	@$(CC) $(CFLAGS) -fPIC -c fstring.c -o fstring.o
	@echo "Linking"
	@$(LD) $(LDFLAGS) -o $(DNAME) fstring.o
	@$(AR) $(ARFLAGS) $(SNAME) fstring.o >/dev/null
	@ldconfig -v -n . >/dev/null
	@echo "Cominging tests"
	@$(CC) $(TEST_CFLAGS) test.c fstring.c -o test

test: build
	./test

docs: 
	doxygen Doxyfile  

clean:
	rm -f fstring test $(SNAME) $(DNAME) $(FNAME).so*
	rm -rf docs/*

.PHONY: docs
