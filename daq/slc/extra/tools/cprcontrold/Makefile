CXX     = g++
#CXX     = g++44
RM      = rm -rf
CP      = cp -f

BELLE2_LIB = $(BELLE2_LOCAL_DIR)/lib/$(BELLE2_SUBDIR)
BELLE2_INC = $(BELLE2_LOCAL_DIR)/include
BELLE2EX_LIB = $(BELLE2_EXTERNALS_DIR)/lib/$(BELLE2_EXTERNALS_SUBDIR)
BELLE2EX_INC = $(BELLE2_EXTERNALS_DIR)/include

BINDIR  = ./bin
SRCDIR  = ./src
INCDIR  = ./include
TMPDIR  = ./tmp
LIBDIR  = ./lib

LIBS    += -lm -lpthread -ldl -lz -lrt
LIBS    += -L./mgt/ -lmgt
LIBS    += -L./ttrx/ -lttrx
LIBS    += -L$(BELLE2_LIB) -ldaq #-ldaq_slc_database
LIBS    += -L$(BELLE2EX_LIB) -lnsm2_corelib -lnsm2_b2lib
LIBS    += $(shell xml2-config --libs)

CFLAGS   = -g -Wall -fPIC -D_REENTRANT 
CFLAGS  += -I./include -I.
CFLAGS  += -I$(BELLE2_INC)
CFLAGS  += -I$(BELLE2EX_INC)
CFLAGS  += $(shell xml2-config --cflags)

MAINS   = $(wildcard *.cc)
TARGETS = $(MAINS:%.cc=$(BINDIR)/%)

SRCS   = $(wildcard $(SRCDIR)/*.cc)
DEPS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.d)
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

.PHONY : all clean mgt ttrx

all: mgt ttrx $(TARGETS)
	@ install -m755 -p $(BINDIR)/* $(BELLE2_LOCAL_DIR)/bin/$(BELLE2_SUBDIR)

mgt: 
	@ make -C mgt

ttrx: 
	@ make -C ttrx

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

