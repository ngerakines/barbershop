## Process this file with automake to produce Makefile.in

bin_PROGRAMS = barbershop
barbershop_SOURCES = barbershop.c barbershop.h bst.c bst.h scores.c scores.h stats.h
barbershop_CFLAGS = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)
barbershop_LDFLAGS = -lm -pthread -levent
