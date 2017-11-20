#
#  Makefile for NSM2 example clients
#

ALL = client client_wait master readdat anonymous bridge nsmget
LIBS      = 
CC        = gcc
CXX       = g++
OPT       = -g -I ../corelib -I ../b2lib
DEPS      = Makefile
EXTRA_CFLAGS =
CFLAGS    = $(OPT) $(EXTRA_CFLAGS)
LIBS = ../corelib/nsmlib2.o \
       ../corelib/nsmhash.o \
       ../corelib/nsmparse.o \
       ../b2lib/belle2nsm.o

.c.o:
	$(CC) $(CFLAGS) -c $<

.cc.o:
	$(CXX) $(CFLAGS) -c $<

all:: $(ALL)

clean::
	-rm -f $(ALL) *.o *.bak *~

client: client.o $(LIBS)
	$(CXX) -o client $(CFLAGS) client.o $(LIBS)

client_wait: client_wait.o $(LIBS)
	$(CXX) -o client_wait $(CFLAGS) client_wait.o $(LIBS)

readdat: readdat.o $(LIBS)
	$(CXX) -o readdat $(CFLAGS) readdat.o $(LIBS)

nsmget: nsmget.o $(LIBS) ../nsmget/libnsmget.o
	$(CXX) -o nsmget $(CXXFLAGS) nsmget.o $(LIBS) ../nsmget/libnsmget.o

anonymous: anonymous.o $(LIBS)
	$(CXX) -o anonymous $(CFLAGS) anonymous.o $(LIBS)

master: master.o $(LIBS)
	$(CXX) -o master $(CFLAGS) master.o $(LIBS) -lreadline -ltermcap

master2: master2.o $(LIBS)
	$(CXX) -o master2 $(CFLAGS) master2.o $(LIBS) -lreadline -ltermcap

bridge: bridge.o $(LIBS)
	$(CXX) -o bridge $(CFLAGS) bridge.o $(LIBS)

client.o:      client.cc $(DEPS)
client_wait.o: client_wait.cc $(DEPS)
readdat.o:     readdat.cc $(DEPS)
anonymous.o:   anonymous.cc $(DEPS)
bridge.o:      bridge.cc $(DEPS)

master.o: master.cc $(DEPS)
	$(CXX) $(CFLAGS) -c -DUSE_READLINE master.cc

