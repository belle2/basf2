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
LIBS    += -L$(BELLE2_LIB) -ldaq_slc_base -ldaq_slc_system -ldaq_slc_database -ldaq_slc_psql -ldaq_slc_nsm -ldaq_slc_runcontrol -ldaq_slc_apps_nsm2socket
LIBS    += -lnsm2_corelib -lnsm2_b2lib
LIBS    += -L$(PGSQL_LIB_PATH) -lpq

CFLAGS   = -g -Wall -fPIC -D_REENTRANT
CFLAGS  += -I$(BELLE2_DAQ_SLC)/include -I.
CFLAGS  += -I$(BELLE2_INC)
ifneq ($(NOT_USE_PSQL),yes)
CFLAGS  += -I$(PGSQL_INC_PATH)
endif

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

