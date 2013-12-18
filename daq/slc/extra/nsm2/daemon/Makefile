#
#  Makefile for NSM2 daemon (nsmd2)
#

BELLE2_SLC_PATH = $(BELLE2_LOCAL_DIR)/daq/slc/bin
ALL  = nsmd2 nsminfo2
CC   = gcc
CXX  = g++
CFLAGS = -g
DEPS = nsm2.h nsmsys2.h Makefile

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

install::
	install nsmd2 $(BELLE2_SLC_PATH)/
	install nsminfo2 $(BELLE2_SLC_PATH)/
#	install nsm2.h $(EXTINCDIR)/nsm2
#	install nsmsys2.h $(EXTINCDIR)/nsm2


clean::
	-rm -f *.o *.bak *~ nsmd2 nsminfo2

nsmd2: nsmd2.o nsmhash.o
	$(CXX) -o nsmd2 $(CFLAGS) nsmd2.o nsmhash.o $(LIBS)

nsminfo2: nsminfo2.o nsmhash.o
	$(CXX) -o nsminfo2 $(CFLAGS) nsminfo2.o nsmhash.o $(LIBS)

nsmd2.o: $(DEPS)
nsminfo2.o: $(DEPS)
nsmhash.o: $(DEPS)

