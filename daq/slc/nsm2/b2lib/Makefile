#
#  Makefile for NSM2 b2lib
#

ALL = main
LIBS      = 
CC        = gcc
CXX       = g++
OPT = -g -fPIC -I../corelib
DEPS   = belle2nsm.h Makefile
CORELIBS = ../corelib/nsmlib2.o ../corelib/nsmhash.o ../corelib/nsmparse.o
CFLAGS = $(OPT) $(EXTRA_CFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

clean::
	-rm -f *.o *.bak *~ $(ALL)

main: main.o belle2nsm.o
	$(CXX) -o main $(CFLAGS) main.o belle2nsm.o $(CORELIBS) $(LIBS)


main.o: $(DEPS)

belle2nsm.o: $(DEPS)
