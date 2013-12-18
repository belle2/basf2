CXX     = g++
RM      = rm -rf
CP      = cp -f

BELLE2_LIB = $(BELLE2_LOCAL_DIR)/daq/slc/lib
BELLE2_INC = $(BELLE2_LOCAL_DIR)/daq/slc/include

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    += -lm -lpthread -lrt -lz
LIBS    += -L$(BELLE2_LIB) -ldaq_slc_base -ldaq_slc_system -ldaq_slc_nsm -ldaq_slc_xml
LIBS    += -L$(BELLE2_LIB) -lnsm2_corelib -lnsm2_b2lib
LIBS    += $(shell xml2-config --libs)
LIBS    += $(shell mysql_config --libs)

CFLAGS   = -g -Wall -fPIC -D_REENTRANT -DB2SLC_DEBUG_MODE #-Wno-deprecated
CFLAGS  += -I./include 
CFLAGS  += -I$(BELLE2_INC)
CFLAGS  += -I$(BELLE2EX_INC)
CFLAGS  += $(shell xml2-config --cflags)
CFLAGS  += $(shell mysql_config --cflags)

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
	@ $(RM) lib/*.so

$(TARGETS): $(BINDIR)/% : %.cc $(OBJS)
	@ echo '<< creating executable $@ >>'
	@ if [ ! -d $(BINDIR) ];then mkdir $(BINDIR) ;fi
	@ $(CXX) $(CFLAGS) $^ -o $@ $(LIBS) $(GLIBS) $(LOADLIBES) 
	@ $(RM) *~ */*~ \#*\#* */\#*\#*
	@ echo '<< compiling succeded!! >>'

$(OBJS): $(TMPDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.h
	@ echo '<< compiling $@ >>'
	@ $(CXX) -c $(CFLAGS) $< -o  $@

$(DEPS) : $(TMPDIR)/%.d: $(SRCDIR)/%.cc $(INCDIR)/%.h
	@ if [ ! -d $(TMPDIR) ];then mkdir $(TMPDIR) ;fi
	@ echo '<< making dipendency $@ >>'
	@ $(CXX) $(CFLAGS) -MM -MT $(basename $@).o $< -o $@

-include $(DEPS)

