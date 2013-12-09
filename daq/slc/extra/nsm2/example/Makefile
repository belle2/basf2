#
#  Makefile for NSM2 example clients
#

ALL = client client_wait master readdat bridge reciever
LIBS      = 
CC        = gcc
CXX       = g++
OPT = -g -I ../corelib -I ../b2lib -lm -lpthread
DEPS   = Makefile
CFLAGS = $(OPT) $(EXTRA_CFLAGS)
LIBS += ../corelib/nsmlib2.o \
	../corelib/nsmhash.o \
        ../corelib/nsmparse.o \
        ../b2lib/belle2nsm.o

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

clean::
	-rm -f $(UNIX_TARGET) $(VXW_TARGET) *.o *.bak *~ $(ALL)

bridge: bridge.o $(LIBS)
	$(CXX) -o bridge $(CFLAGS) bridge.o $(LIBS)

reciever: reciever.o $(LIBS)
	$(CXX) -o reciever $(CFLAGS) reciever.o $(LIBS)

client: client.o $(LIBS)
	$(CXX) -o client $(CFLAGS) client.o $(LIBS)

client_wait: client_wait.o $(LIBS)
	$(CXX) -o client_wait $(CFLAGS) client_wait.o $(LIBS)

readdat: readdat.o $(LIBS)
	$(CXX) -o readdat $(CFLAGS) readdat.o $(LIBS)

master: master.o $(LIBS)
	$(CXX) -o master $(CFLAGS) master.o $(LIBS)

reciever.o: $(DEPS)
bridge.o: $(DEPS)
client.o: $(DEPS)
client_wait.o: $(DEPS)
readdat.o: $(DEPS)

master.o: $(DEPS)
	$(CXX) $(CFLAGS) -c master.cc #-DUSE_READLINE master.cc 
