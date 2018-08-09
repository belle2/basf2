#
#  Makefile for NSM2 corelib
#

ALL = nsmlib2.o nsmparse.o nsmhash.o
LIBS      = 
CC        = gcc
CXX       = g++
OPT = -g
DEPS   = nsm2.h nsmsys2.h nsmlib2.h Makefile
EXTRA_CFLAGS = -shared -fPIC
CFLAGS = $(OPT) $(EXTRA_CFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL) libs

libs::
	ar r libnsm2_corelib.a $(ALL)
	g++ -shared -o libnsm2_corelib.so $(ALL)

install::
	install libnsm2_corelib.a $(EXTLIBDIR)
	install libnsm2_corelib.so $(EXTLIBDIR)
	mkdir $(EXTINCDIR)/nsm2
	install nsmlib2.h $(EXTINCDIR)/nsm2

clean::
	-rm -f *.o *.a *.so *.bak *~

testparse::
	$(CC) $(OPT) -DTESTPARSE -o testparse nsmparse.c

nsmlib2.o: $(DEPS)

nsmhash.o: $(DEPS)

nsmparse.o: $(DEPS) nsmparse.h

