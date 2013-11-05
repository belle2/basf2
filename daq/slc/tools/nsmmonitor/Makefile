CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

NSM2_PATH  = $(BELLE2_DAQ_SLC_DIR)/nsm2

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    +=  -lm -lpthread -lz 
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_base
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_nsm
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -lnsm2_corelib
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -lnsm2_b2lib
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_database
LIBS    += $(shell mysql_config --libs)

CFLAGS   = -g -Wall -fPIC -D_REENTRANT 
CFLAGS  += -I./include 
CFLAGS  += -I$(BELLE2_DAQ_SLC_DIR)/include 
CFLAGS  += -I$(BELLE2_DAQ_SLC_DIR)/data/nsm
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

$(OBJS): $(TMPDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.hh
	@ echo '<< compiling $@ >>'
	@ $(CXX) -c $(CFLAGS) $< -o  $@

$(DEPS) : $(TMPDIR)/%.d: $(SRCDIR)/%.cc $(INCDIR)/%.hh
	@ if [ ! -d $(TMPDIR) ];then mkdir $(TMPDIR) ;fi
	@ echo '<< making dipendency $@ >>'
	@ $(CXX) $(CFLAGS) -MM -MT $(basename $@).o $< -o $@

-include $(DEPS)

