#
#  Makefile for NSM2 example clients
#

BELLE2_SLC_PATH  = $(BELLE2_LOCAL_DIR)/daq/slc

ALL     = shmclean
CC      = gcc
CXX     = g++
CFLAGS  = -g 
LIBS    = -lrt

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

clean::
	-rm -f shmclean *.o *~ \#*~

shmclean: shmclean.o 
	$(CXX) -o shmclean $(CFLAGS) shmclean.o $(LIBS)

shmclean.o: 
	$(CXX) $(CFLAGS) -c -DUSE_READLINE shmclean.cc
