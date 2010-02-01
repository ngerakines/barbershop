# Barbershop Makefile
# Copyright (C) 2010 Nick Gerakines <nick at gerakines dot net>
# This file is released under the MIT license.

CC=gcc
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
OPTIMIZATION?=-O2
ifeq ($(uname_S),SunOS)
  CFLAGS?= -std=c99 -pedantic $(OPTIMIZATION) -Wall -W -D__EXTENSIONS__ -D_XPG6
  CCLINK?= -ldl -lnsl -lsocket -lm -lpthread
else
  CFLAGS?= $(OPTIMIZATION) -Wall $(ARCH) $(PROF)
  CCLINK?= -lm -pthread -levent
endif
CCOPT= $(CFLAGS) $(CCLINK) $(ARCH) $(PROF)
DEBUG?= -g -rdynamic -ggdb

OBJ = barbershop.o bst.o scores.o
CLIOBJ = client.o

PRGNAME = barbershop
CLIPRGNAME = client

all: barbershop

# Deps (use make dep to generate this)
client.o: client.c
barbershop.o: barbershop.c stats.h
bst.o: bst.c bst.h stats.h
scores.o: scores.c scores.h stats.h
benchmark.o: benchmark.c

barbershop: $(OBJ)
	$(CC) -o $(PRGNAME) $(CCOPT) $(DEBUG) $(OBJ)

client: $(CLIOBJ)
	$(CC) -o $(CLIPRGNAME) $(CCOPT) $(DEBUG) $(CLIOBJ)

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUG) $(COMPILE_TIME) $<

clean:
	rm -rf $(PRGNAME) $(BENCHPRGNAME) $(CLIPRGNAME) *.o *.gcda *.gcno *.gcov

dep:
	$(CC) -MM *.c

staticsymbols:
	tclsh utils/build-static-symbols.tcl > staticsymbols.h

log:
	git log '--pretty=format:%ad %s' --date=short > Changelog

32bit:
	make ARCH="-arch i386"

gprof:
	make PROF="-pg"

gcov:
	make PROF="-fprofile-arcs -ftest-coverage"

noopt:
	make OPTIMIZATION=""

32bitgprof:
	make PROF="-pg" ARCH="-arch i386"
