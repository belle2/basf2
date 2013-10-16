#
#  Makefile for NSM2 corelib
#

ALL = nsmlib2.o nsmparse.o nsmhash.o
LIBS      = 
CC        = gcc
CXX       = g++
OPT = -g -fPIC
DEPS   = nsm2.h nsmsys2.h nsmlib2.h Makefile
CFLAGS = $(OPT) $(EXTRA_CFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

clean::
	-rm -f *.o *.bak *~

nsmlib2.o: $(DEPS)

nsmhash.o: $(DEPS)

nsmparse.o: $(DEPS)

