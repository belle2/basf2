#
#  Makefile for NSM2 daemon (nsmd2)
#

ALL = nsmd2 nsminfo2
LIBS      = 
CC        = gcc
CXX       = g++
OPT = -g 
DEPS   = nsm2.h nsmsys2.h Makefile
CFLAGS = $(OPT) $(EXTRA_CFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

clean::
	-rm -f *.o *.bak *~ $(ALL)

nsmd2: nsmd2.o nsmhash.o
	$(CXX) -o nsmd2 $(CFLAGS) nsmd2.o nsmhash.o $(LIBS)

nsminfo2: nsminfo2.o nsmhash.o
	$(CXX) -o nsminfo2 $(CFLAGS) nsminfo2.o nsmhash.o $(LIBS)

nsmd2.o: $(DEPS)
nsminfo2.o: $(DEPS)
nsmhash.o: $(DEPS)

