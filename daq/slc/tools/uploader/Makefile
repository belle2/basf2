CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_base
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_system
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_xml
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_database
LIBS    += -lm -lpthread
LIBS    += $(shell xml2-config --libs)
LIBS    += $(shell mysql_config --libs)

CFLAGS   = -g -Wall -fPIC -D_REENTRANT -DB2SLC_DEBUG_MODE #-Wno-deprecated
CFLAGS  += -I./include
CFLAGS  += -I$(BELLE2_DAQ_SLC_DIR)/include
CFLAGS  += $(GTKCFLAGS)
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

