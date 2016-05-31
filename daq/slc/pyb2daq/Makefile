CXX     = g++
CP      = cp
MAKE    = make
MKDIR   = mkdir -p
AR      = ar rcsv
RM      = rm -f

TARGET = python

SLCDIR   = ..
SRCDIR   = ./src
INCDIR   = ./include
TMPDIR   = $(SLCDIR)/tmp
LIBDIR   = $(SLCDIR)/lib

#LIBNAME    = daq_slc_$(shell echo $(TARGET) | sed "s/\//_/g")
LIBNAME    = pyb2daq
LIBTARGET  = $(LIBDIR)/$(LIBNAME).so

SRCS     = $(wildcard $(SRCDIR)/*.cc)
INCS     = $(wildcard $(INCDIR)/*.h)
OBJS     = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)
DEPS     = $(OBJS:%.o=%.d)

CFLAGS   = -g -fPIC -O2 -Wall -DPYHSLB
CFLAGS  += -I$(SLCDIR)/include
ifeq ($(NOT_USE_PSQL),yes)
CFLAGS  += -DNOT_USE_PSQL
else 
CFLAGS  += -I$(PGSQL_INC_PATH)
endif
CFLAGS  += $(EXTERNAL_CFLAGS)

EX_LIB = $(BELLE2_EXTERNALS_DIR)/lib
EX_INC = $(BELLE2_EXTERNALS_DIR)/include
#EX_LIB = $(B2DAQ_EXTERNALS_DIR)/lib
#EX_INC = $(B2DAQ_EXTERNALS_DIR)/include
CFLAGS += -isystem/$(EX_INC)/python3.5 -isystem/$(EX_INC) -I$(EX_INC)/python3.5

LIBS   += -L$(EX_LIB) -lboost_python -lboost_filesystem -lboost_program_options -lboost_system -lpython3.5

LIBS    += -lm -lpthread -ldl -lrt \
         -L$(SLCDIR)/lib -ldaq_slc_base -ldaq_slc_system -ldaq_slc_database -ldaq_slc_psql \
	 -ldaq_slc_nsm -ldaq_slc_runcontrol -ldaq_slc_readout -ldaq_slc_copper \
	 -lhs -lttrx -lnsm2_corelib -lnsm2_b2lib
LIBS    += -L$(PGSQL_LIB_PATH) -lpq

.PHONY : all clean

all: inc $(DEPS) $(LIBTARGET)
clean:
	@ echo "<< cleaning all monitor system >>"
	@ $(RM) *~ */*~ */*/*~ \#*\#* */\#*\#*

inc:
	@ mkdir -p $(SLCDIR)/include/daq/slc/pyb2daq; install -m 644 -p $(INCDIR)/*.h $(SLCDIR)/include/daq/slc/pyb2daq

$(LIBTARGET) : $(OBJS)
	@ echo "<< creating shared librariy $@ >>"
	 $(CXX) -shared  $(CFLAGS) $^ -o $@ $(LIBS)
	@ echo "<< compiling succeede!! >>"
	@ $(RM) *~ */*~ */*/*~ */*/*/*~ */*/*/*/*~ \#*\#* */\#*\#*

$(OBJS) : $(TMPDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.h
	@ echo "<< compiling $@ >>"
	 $(CXX) -c $(CFLAGS) $< -o $@ 

$(DEPS) : $(TMPDIR)/%.d: $(SRCDIR)/%.cc $(INCDIR)/%.h
	@ $(MKDIR) $(TMPDIR)
	@ echo "<< making dependency $@ >>"
	@ $(CXX) $(CFLAGS) -MM -MT $(basename $@).o $< -o $@

-include $(DEPS)
all:

