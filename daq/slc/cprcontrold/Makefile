CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

B2SLC_LIB  = $(B2SLC_PATH)
NSM2_PATH  = $(B2SLC_PATH)/nsm2
MGT_PATH   = $(B2SLC_PATH)/cprcontrold/mgt

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    += -lm -lpthread -ldl -lz -lrt
LIBS    += $(MGT_PATH)/*.o
#LIBS    += -L$(MGT_PATH) -lMGT
LIBS    += -L$(B2SLC_LIB)/lib -ldaq_slc_base
LIBS    += -L$(B2SLC_LIB)/lib -ldaq_slc_system
LIBS    += -L$(B2SLC_LIB)/lib -ldaq_slc_nsm
LIBS    += $(NSM2_PATH)/corelib/nsmlib2.o \
           $(NSM2_PATH)/corelib/nsmhash.o \
           $(NSM2_PATH)/corelib/nsmparse.o \
           $(NSM2_PATH)/b2lib/belle2nsm.o

CFLAGS   = -g -Wall -fPIC -D_REENTRANT 
CFLAGS  += -I./include 
CFLAGS  += -I$(MGT_PATH)
CFLAGS  += -I$(NSM2_PATH)/corelib -I$(NSM2_PATH)/b2lib
CFLAGS  += -I$(B2SLC_LIB)/include 

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

