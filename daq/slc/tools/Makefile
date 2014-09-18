CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

BELLE2_LIB = $(BELLE2_DAQ_SLC)/lib
BELLE2_INC = $(BELLE2_DAQ_SLC)/include
BELLE2EX_LIB = $(BELLE2_EXTERNAL_LOCAL_DIR)/lib/$(shell echo `uname -s`_`uname -m`)/opt
BELLE2EX_INC = $(BELLE2_EXTERNAL_LOCAL_DIR)/include

BINDIR  = $(BELLE2_DAQ_SLC)/bin
SRCDIR  = $(BELLE2_DAQ_SLC)/src
INCDIR  = $(BELLE2_DAQ_SLC)/include
TMPDIR  = $(BELLE2_DAQ_SLC)/tmp
LIBDIR  = $(BELLE2_DAQ_SLC)/lib

LIBS    += -lm -lpthread -ldl -lz -lrt -lpq -lreadline -lncurses -L/usr/pgsql-9.2/lib -lpq
LIBS    += -L$(BELLE2_LIB) -ldaq_slc_base -ldaq_slc_system -ldaq_slc_nsm -ldaq_slc_database
LIBS    += -L$(BELLE2EX_LIB) -lnsm2_corelib -lnsm2_b2lib

CFLAGS   = -g -Wall -fPIC -D_REENTRANT -I/usr/pgsql-9.2/include
CFLAGS  += -I$(BELLE2_DAQ_SLC)/include -I.
CFLAGS  += -I$(BELLE2_INC)
CFLAGS  += -I$(BELLE2EX_INC)

MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

.PHONY : all clean 

all: $(TARGETS)
#	@ install -m755 -p $(BINDIR)/* $(BELLE2_LOCAL_DIR)/bin/$(BELLE2_SUBDIR)
#	@ install -m755 -p $(BINDIR)/* $(BELLE2_LOCAL_DIR)/bin

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

