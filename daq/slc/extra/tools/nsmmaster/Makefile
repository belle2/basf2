#
#  Makefile for NSM2 example clients
#

BELLE2_SLC_PATH  = $(BELLE2_LOCAL_DIR)/daq/slc

ALL     = cui
CC      = gcc
CXX     = g++
CFLAGS  = -g -I$(BELLE2_SLC_PATH)/include -I$(BELLE2_SLC_PATH)/include
LIBS    = -L$(BELLE2_SLC_PATH)/lib -lnsm2_corelib -lnsm2_b2lib

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

clean::
	-rm -f cui *.o *~ \#*~

cui: cui.o 
	$(CXX) -o cui $(CFLAGS) cui.o $(LIBS) -lreadline -lcurses
	install -m755 -p $@ $(BELLE2_LOCAL_DIR)/daq/slc/bin

cui.o: 
	$(CXX) $(CFLAGS) -c -DUSE_READLINE cui.cc
