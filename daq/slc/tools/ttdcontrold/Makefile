CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

NSM2_PATH  = $(BELLE2_DAQ_SLC_DIR)/nsm2
FTSW_PATH  = $(BELLE2_DAQ_SLC_DIR)/ttdcontrold/ftsw

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    +=  -lm -lpthread -lz -lvme 
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_base
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_system
LIBS    += -L$(BELLE2_DAQ_SLC_DIR)/lib -ldaq_slc_nsm
LIBS    += $(FTSW_PATH)/ftswlib.o
LIBS    += $(NSM2_PATH)/corelib/nsmlib2.o \
           $(NSM2_PATH)/corelib/nsmhash.o \
           $(NSM2_PATH)/corelib/nsmparse.o \
           $(NSM2_PATH)/b2lib/belle2nsm.o

CFLAGS   = -g -Wall -fPIC -D_REENTRANT 
CFLAGS  += -I./include 
CFLAGS  += -I$(FTSW_PATH)
CFLAGS  += -I$(NSM2_PATH)/corelib -I$(NSM2_PATH)/b2lib
CFLAGS  += -I$(BELLE2_DAQ_SLC_DIR)/include 

MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

.PHONY : all clean ftsw

all: ftsw $(TARGETS)

ftsw:
	@ make -C ftsw/

force:
	@ $(RM) $(BINDIR)/*
	@ make all

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) $(BINDIR) $(TMPDIR)
	@ make -C ftsw clean
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

