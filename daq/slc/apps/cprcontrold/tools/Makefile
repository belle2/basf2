CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

BELLE2_LIB = $(BELLE2_DAQ_SLC)/lib
BELLE2_INC = $(BELLE2_DAQ_SLC)/include

BINDIR  = $(BELLE2_DAQ_SLC)/bin
SRCDIR  = $(BELLE2_DAQ_SLC)/src
INCDIR  = $(BELLE2_DAQ_SLC)/include
TMPDIR  = $(BELLE2_DAQ_SLC)/tmp
LIBDIR  = $(BELLE2_DAQ_SLC)/lib

LIBS    += -lm -lpthread -ldl -lrt
LIBS    += -L$(BELLE2_LIB) -ldaq_slc_base -ldaq_slc_system -ldaq_slc_database\
 -ldaq_slc_nsm -ldaq_slc_runcontrol -ldaq_slc_readout -ldaq_slc_copper \
 -ldaq_slc_apps_cprcontrold -lhs -lttrx -ldaq_slc_pyb2daq
LIBS    += -lnsm2_corelib -lnsm2_b2lib
LIBS   += -L$(BELLE2_DAQ_SLC)/externals/boost/lib -lboost_python -lboost_filesystem -lboost_program_options -lboost_system
LIBS   += -L$(BELLE2_DAQ_SLC)/externals/python/lib -lpython3.5m
LIBS    += -lnsm2_corelib -lnsm2_b2lib

CFLAGS   = -g -Wall -fPIC -D_REENTRANT
CFLAGS  += -I$(BELLE2_DAQ_SLC)/include -I.
CFLAGS  += -I$(BELLE2_DAQ_SLC)/externals/python/include/python3.5m 
CFLAGS  += -I$(BELLE2_DAQ_SLC)/externals/boost/include

MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

.PHONY : all clean 

all: $(TARGETS)

force:
	@ $(RM) $(BINDIR)/*
	@ make all

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) *~ */*~ \#*\#* */\#*\#*

$(TARGETS): $(BINDIR)/% : %.cc $(OBJS)
	@ echo '<< creating executable $@ >>'
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

