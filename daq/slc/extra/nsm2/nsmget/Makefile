#
#  Makefile for NSM2 b2nsmaccess
#

ALL = libnsmget.o
LIBS      = 
CC        = gcc
CXX       = g++
OPT = -g
DEPS   = nsmget.h Makefile
EXTRA_CFLAGS = -shared -fPIC
CFLAGS = $(OPT) $(EXTRA_CFLAGS) -I../corelib/
CXXFLAGS = $(OPT) $(EXTRA_CFLAGS) -I../corelib/

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL) libs

libs::
	ar r libnsmget.a $(ALL)
	g++ -shared -o libnsmget.so $(ALL)

install::
	install libnsmget.a $(EXTLIBDIR)
	install libnsmget.so $(EXTLIBDIR)
	mkdir $(EXTINCDIR)/nsm2
	install nsmget.h $(EXTINCDIR)/nsm2

clean::
	-rm -f *.o *.a *.so *.bak *~

b2nsmaccess.o: $(DEPS)

nsmparse.o: $(DEPS) ../corelib/nsmparse.h


