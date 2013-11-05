#
#  Makefile for NSM2 b2lib
#

ALL      = main
LIBS     = 
CC       = gcc
CXX      = g++
OPT      = -g -I../corelib
DEPS     = belle2nsm.h Makefile
CORELIBS = ../corelib/nsmlib2.o ../corelib/nsmhash.o ../corelib/nsmparse.o
CFLAGS   = $(OPT) -fPIC
INSDIR   = $(BELLE2_DAQ_SLC_DIR)

.c.o:
	@ echo '<< creating executable $@ >>'
	@ $(CC) $(CFLAGS) -c $<

.cc.o:
	@ echo '<< creating executable $@ >>'
	@ $(CXX) $(CFLAGS) -c $<

all:: libs

clean::
	@ -rm -f *.o *.a *.so *~

main: main.o belle2nsm.o
	@ $(CXX) -o main $(CFLAGS) main.o belle2nsm.o $(CORELIBS) $(LIBS)

libs:: 	belle2nsm.o
	@ g++ -shared -o libnsm2_b2lib.so belle2nsm.o

install::
	@ install libnsm2_b2lib.so $(INSDIR)/lib
	@ mkdir -p $(INSDIR)/include/nsm
	@ install -m644 -p belle2nsm.h $(INSDIR)/include/nsm

main.o: $(DEPS)

belle2nsm.o: $(DEPS)
