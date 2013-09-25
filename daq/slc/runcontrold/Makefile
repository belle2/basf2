CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

B2SLC_LIB  = $(B2SLC_PATH)
NSM2_PATH  = $(B2SLC_PATH)/nsm2

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    += -L$(B2SLC_LIB)/lib -lB2SLCUTIL
LIBS    += -L$(B2SLC_LIB)/lib -lB2SLCNODE
LIBS    += -L$(B2SLC_LIB)/lib -lB2SLCSYS
LIBS    += -L$(B2SLC_LIB)/lib -lB2SLCNSM
LIBS    += -L$(B2SLC_LIB)/lib -lB2SLCXML
LIBS    += -L$(B2SLC_LIB)/lib -lB2SLCDB
LIBS    += -L$(B2SLC_LIB)/lib -lB2SLCRC
LIBS    += $(NSM2_PATH)/corelib/nsmlib2.o \
           $(NSM2_PATH)/corelib/nsmhash.o \
           $(NSM2_PATH)/corelib/nsmparse.o \
           $(NSM2_PATH)/b2lib/belle2nsm.o
LIBS    += -lm -lpthread -lz -ldl
LIBS    += $(shell xml2-config --libs)
LIBS    += $(shell mysql_config --libs)

CFLAGS   = -g -Wall -fPIC -D_REENTRANT -DB2SLC_DEBUG_MODE #-Wno-deprecated
CFLAGS  += -I./include
CFLAGS  += -I$(NSM2_PATH)/corelib -I$(NSM2_PATH)/b2lib
CFLAGS  += -I$(B2SLC_LIB)/include
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

$(OBJS): $(TMPDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.hh
	@ echo '<< compiling $@ >>'
	@ $(CXX) -c $(CFLAGS) $< -o  $@

$(DEPS) : $(TMPDIR)/%.d: $(SRCDIR)/%.cc $(INCDIR)/%.hh
	@ if [ ! -d $(TMPDIR) ];then mkdir $(TMPDIR) ;fi
	@ echo '<< making dipendency $@ >>'
	@ $(CXX) $(CFLAGS) -MM -MT $(basename $@).o $< -o $@

-include $(DEPS)

