#
#  Makefile for NSM2 daemon (nsmd2)
#

ALL       = nsmd2 nsminfo2
LIBS      = 
CC        = gcc
CXX       = g++
OPT       = -g
DEPS      = nsm2.h nsmsys2.h Makefile
EXTRA_CFLAGS =
CFLAGS    = $(OPT) $(EXTRA_CFLAGS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

install::
	install nsmd2 $(EXTBINDIR)
	install nsminfo2 $(EXTBINDIR)
	install nsm2.h $(EXTINCDIR)/nsm2
	install nsmsys2.h $(EXTINCDIR)/nsm2
clean::
	-rm -f $(ALL) *.o *.bak *~

nsmd2: nsmd2.o nsmhash.o nsmd2info.o
	$(CXX) -o nsmd2 $(CFLAGS) nsmd2.o nsmhash.o nsmd2info.o $(LIBS)

nsminfo2: nsminfo2.o nsmhash.o
	$(CXX) -o nsminfo2 $(CFLAGS) nsminfo2.o nsmhash.o $(LIBS)

nsmd2.o: nsmd2.cc $(DEPS)
nsminfo2.o: nsminfo2.cc $(DEPS)
nsmhash.o: nsmhash.c $(DEPS)

nsmd2info.o: nsminfo2.cc $(DEPS)
	$(CXX) $(CFLAGS) -DNSMD2INFO -o nsmd2info.o -c nsminfo2.cc
