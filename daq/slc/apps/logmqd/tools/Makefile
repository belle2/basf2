CXX     = g++
RM      = rm -rf
CP      = cp -f

BINDIR  = $(BELLE2_DAQ_SLC)/bin
SRCDIR  = $(BELLE2_DAQ_SLC)/src
INCDIR  = $(BELLE2_DAQ_SLC)/include
TMPDIR  = $(BELLE2_DAQ_SLC)/tmp
LIBDIR  = $(BELLE2_DAQ_SLC)/lib

LIBS    += -lm -lpthread -ldl -lrt #-lreadline
LIBS    += -L$(LIBDIR) -ldaq_slc_base -ldaq_slc_system \
-ldaq_slc_database -ldaq_slc_psql -ldaq_slc_nsm \
-ldaq_slc_apps_logmqd
LIBS    += -lnsm2_corelib -lnsm2_b2lib
LIBS    += -L$(PGSQL_LIB_PATH) -lpq
LIBS    += -L$(BELLE2_DAQ_SLC)/externals/activemq/lib -lactivemq-cpp
LIBS    += -L$(BELLE2_DAQ_SLC)/externals/apr/lib -lapr-1

CMS_INC=$(BELLE2_DAQ_SLC)/externals/activemq/include/activemq-cpp-3.9.3
APR_INC=$(BELLE2_DAQ_SLC)/externals/apr/include/apr-1

CFLAGS   = -g -Wall -fPIC -D_REENTRANT
CFLAGS  += -I$(BELLE2_DAQ_SLC)/include -I.
ifneq ($(NOT_USE_PSQL),yes)
CFLAGS  += -I$(PGSQL_INC_PATH) -lpq
endif

CFLAGS  += -I$(APR_INC)
CFLAGS  += -I$(CMS_INC)

MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)

OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

.PHONY : all clean 

all: $(TARGETS)

force:
	@ $(RM) $(TARGETS)
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

