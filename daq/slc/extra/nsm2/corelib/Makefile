#
#  Makefile for NSM2 corelib
#

ALL    = nsmlib2.o nsmparse.o nsmhash.o
LIBS   = 
CC     = gcc
CXX    = g++
OPT    = -g
DEPS   = nsm2.h nsmsys2.h nsmlib2.h Makefile
CFLAGS = $(OPT) -fPIC
INSDIR = $(BELLE2_DAQ_SLC_DIR)

.c.o:
	@ echo '<< creating executable $@ >>'
	@ $(CC) $(CFLAGS) -c $<

.cc.o:
	@ echo '<< creating executable $@ >>'
	@ $(CXX) $(CFLAGS) -c $<

all:: $(ALL) libs

libs::
	@ $(CXX) -shared -o libnsm2_corelib.so $(ALL)

install::
	@ install libnsm2_corelib.so $(INSDIR)/lib
	@ install -m644 -p nsm2.h nsmlib2.h nsmsys2.h $(INSDIR)/include/nsm

clean::
	@ rm -f *.o *.bak *.a *.so *~

nsmlib2.o: $(DEPS)

nsmhash.o: $(DEPS)

nsmparse.o: $(DEPS)

