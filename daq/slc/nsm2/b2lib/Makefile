#
#  Makefile for NSM2 b2lib
#

ALL = belle2nsm.o
LIBS      = 
CC        = gcc
CXX       = g++
OPT       = -g -I../corelib
COREDEPS  = ../corelib/nsm2.h ../corelib/nsmlib2.h ../corelib/nsmsys2.h
DEPS      = belle2nsm.h Makefile $(COREDEPS)
CORELIBS  = ../corelib/nsmlib2.o ../corelib/nsmhash.o ../corelib/nsmparse.o
EXTRA_CFLAGS = -shared -fPIC
CFLAGS    = $(OPT) $(EXTRA_CFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL) libs

libs::
	ar r libnsm2_b2lib.a belle2nsm.o
	g++ -shared -o libnsm2_b2lib.so belle2nsm.o

install::
	install libnsm2_b2lib.a $(EXTLIBDIR)
	install libnsm2_b2lib.so $(EXTLIBDIR)
	install belle2nsm.h $(EXTINCDIR)/nsm2

clean::
	-rm -f *.o *.a *.so *.bak *~ main

main: main.o belle2nsm.o
	$(CXX) -o main $(CFLAGS) main.o belle2nsm.o $(CORELIBS) $(LIBS)

main.o: $(DEPS)

belle2nsm.o: $(DEPS)
