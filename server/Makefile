# Makefile for 'server'
# CS50 project 'Nuggets'
#
# Plankton, May 2023

OBJS = server.o 
LIBS = ../common/common.a ../libs/libs.a ../support/support.a


CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -g -ggdb -I ../common -I ../libs -I ../support -lm
MAKE = make

# for memory-leak tests
VALGRIND = valgrind -s --leak-check=full --show-leak-kinds=all

.PHONY: all test valgrind clean

all: server

server: $(OBJS) $(LIBS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ -lm

server.o: server.c ../common/grid.h ../common/game.h 

clean:
	rm -rf *.dSYM  # MacOS debugger info
	rm -f *~ *.o
	rm -f server
	rm -f *.log

