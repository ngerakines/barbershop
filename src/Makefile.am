## Process this file with automake to produce Makefile.in

bin_PROGRAMS = barbershop client benchmark
barbershop_SOURCES = barbershop.c barbershop.h bst.c bst.h scores.c scores.h stats.h
barbershop_CFLAGS = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)

client_SOURCES = client.c
client_CFLAGS = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)

benchmark_SOURCES = benchmark.c
benchmark_CFLAGS = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)
