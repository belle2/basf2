CXX     = g++
RM      = rm -rf
CP      = cp -f

BINDIR  = $(BELLE2_DAQ_SLC)/bin
SRCDIR  = $(BELLE2_DAQ_SLC)/src
INCDIR  = $(BELLE2_DAQ_SLC)/include
TMPDIR  = $(BELLE2_DAQ_SLC)/tmp
LIBDIR  = $(BELLE2_DAQ_SLC)/lib

LIBS    += -lm -lpthread -ldl -lrt -lreadline -lncurses -lz -L$(PGSQL_LIB_PATH) -lpq
LIBS    += -L$(LIBDIR) -ldaq_slc_base -ldaq_slc_system -ldaq_slc_database -ldaq_slc_psql -ldaq_slc_nsm -ldaq_slc_hvcontrol -ldaq_slc_hvcontrol_arich
LIBS    += -lnsm2_corelib -lnsm2_b2lib -lcaenhvwrapper 

CFLAGS   = -g -Wall -fPIC -D_REENTRANT -I$(PGSQL_INC_PATH)
CFLAGS  += -I$(BELLE2_DAQ_SLC)/include -I.

MAINS   = arichhvd.cc make_db.cc
#MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

.PHONY : all clean 

all: $(TARGETS)
#	@ install -m755 -p $(BINDIR)/* $(BELLE2_LOCAL_DIR)/bin/$(BELLE2_SUBDIR)

force:
	@ $(RM) $(BINDIR)/*
	@ make all

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) *~ */*~ \#*\#* */\#*\#*

$(TARGETS): $(BINDIR)/% : %.cc $(OBJS)
	@ echo '<< creating executable $@ >>'
	 $(CXX) $(CFLAGS) $(LIBS)  $^ -o $@ $(GLIBS) $(LOADLIBES)
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

