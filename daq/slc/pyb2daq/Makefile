CXX     = g++
CP      = cp
MAKE    = make
MKDIR   = mkdir -p
AR      = ar rcsv
RM      = rm -f

TARGET = python

SLCDIR   = $(BELLE2_DAQ_SLC)
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
CFLAGS  += -I$(BELLE2_DAQ_SLC)/externals/boost/include
CFLAGS  += -I$(BELLE2_DAQ_SLC)/externals/python/include/python3.5m

LIBS   += -L$(BELLE2_DAQ_SLC)/externals/boost/lib -lboost_python -lboost_filesystem -lboost_program_options -lboost_system
LIBS   += -L$(BELLE2_DAQ_SLC)/externals/python/lib -lpython3.5m

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
	@ ln -s $@ $(LIBDIR)/libdaq_slc_pyb2daq.so
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

