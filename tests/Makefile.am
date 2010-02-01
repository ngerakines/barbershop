## Process this file with automake to produce Makefile.in

TESTS = check_barbershop
check_PROGRAMS = check_barbershop
check_barbershop_SOURCES = check_barbershop.c $(top_builddir)/src/scores.c $(top_builddir)/src/scores.h
check_barbershop_CFLAGS = @CHECK_CFLAGS@ -g -Wall -fprofile-arcs -ftest-coverage
check_barbershop_LDADD = @CHECK_LIBS@
