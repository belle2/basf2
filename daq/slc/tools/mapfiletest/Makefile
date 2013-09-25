CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    += -lm -lpthread -lrt -lNew
LIBS    += $(shell root-config --libs)

CFLAGS   = -g -Wall -fPIC -D_REENTRANT -DB2DAQ_DEBUG_MODE #-Wno-deprecated
CFLAGS  += -I./include
CFLAGS  += $(shell root-config --cflags)

MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

all: $(TARGETS)

force:
	@ $(RM) $(BINDIR)/*
	@ make all

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) $(BINDIR) $(TMPDIR)
	@ $(RM) *~ */*~ \#*\#* */\#*\#*

$(TARGETS): $(BINDIR)/% : %.cc $(OBJS)
	@ echo '<< creating executable $@ >>'
	@ if [ ! -d $(BINDIR) ];then mkdir $(BINDIR) ;fi
	@ $(CXX) $(CFLAGS) $^ -o $@ $(LIBS) $(GLIBS) $(LOADLIBES) 
	@ $(RM) *~ */*~ \#*\#* */\#*\#*
	@ echo '<< compiling succeded!! >>'

$(OBJS): $(TMPDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.hh
	@ echo '<< compiling $@ >>'
	@ $(CXX) -c $(CFLAGS) $< -o  $@

$(DEPS) : $(TMPDIR)/%.d: $(SRCDIR)/%.cc $(INCDIR)/%.hh
	@ if [ ! -d $(TMPDIR) ];then mkdir $(TMPDIR) ;fi
	@ echo '<< making dipendency $@ >>'
	@ $(CXX) $(CFLAGS) -MM -MT $(basename $@).o $< -o $@

-include $(DEPS)

