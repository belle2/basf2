CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    += -lm -lpthread -ldl -lz -lrt
LIBS    += ./mgt/*.o
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_base
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_system
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_nsm
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_system
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_xml
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -lnsm2_corelib
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -lnsm2_b2lib
LIBS    += $(shell xml2-config --libs)

CFLAGS   = -g -Wall -fPIC -D_REENTRANT 
CFLAGS  += -I./include -I.
CFLAGS  += -I$(BELLE2_DAQ_SLC_DIR)/include 
CFLAGS  += $(shell xml2-config --cflags)

MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

.PHONY : all clean mgt ttrx

all: mgt $(TARGETS)

ttrx:
	@ make -C ttrx

mgt: 
	@ make -C mgt

force:
	@ $(RM) $(BINDIR)/*
	@ make all

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) $(BINDIR) $(TMPDIR)
	@ make -C mgt/ clean
	@ make -C ttrx/ clean
	@ $(RM) *~ */*~ \#*\#* */\#*\#*

$(TARGETS): $(BINDIR)/% : %.cc $(OBJS)
	@ echo '<< creating executable $@ >>'
	@ if [ ! -d $(BINDIR) ];then mkdir $(BINDIR) ;fi
	@ $(CXX) $(CFLAGS) $(LIBS)  $^ -o $@ $(GLIBS) $(LOADLIBES)
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

