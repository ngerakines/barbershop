## Process this file with automake to produce Makefile.in

bin_PROGRAMS = barbershop barbershop-client barbershop-benchmark
barbershop_SOURCES = barbershop.c barbershop.h stats.h commands.c commands.h pqueue.c pqueue.h
barbershop_CFLAGS = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)

barbershop_client_SOURCES = client.c
barbershop_client_CFLAGS = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)

barbershop_benchmark_SOURCES = benchmark.c
barbershop_benchmark_CFLAGS = $(OPTIMIZATION) -Wall $(ARCH) $(PROF)
