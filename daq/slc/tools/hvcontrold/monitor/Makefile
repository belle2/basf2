USER_CLASS=ArichHV
DQM_LIB_PATH=/home/tkonno/belle2/release/daq/slc/data/dqm/lib
CXX     = g++

HOMEDIR  = .
SRCDIR   = $(HOMEDIR)/src
INCDIR   = $(HOMEDIR)/include
BINDIR   = $(HOMEDIR)/bin
TMPDIR   = $(HOMEDIR)/tmp
LIBDIR   = $(HOMEDIR)/lib

NSM2_PATH = $(BELLE2_DAQ_SLC_DIR)/nsm2

SRCS   = $(SRCDIR)/$(USER_CLASS)Monitor.cc $(SRCDIR)/lib$(USER_CLASS)Monitor.cc 
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

CFLAGS   = -g -fPIC -O2 -Wall
CFLAGS  += -I$(INCDIR)
CFLAGS  += -I$(BELLE2_DAQ_SLC_DIR)/include

LIBTARGET  = $(LIBDIR)/libdaq_slc_dqm_$(USER_CLASS).so

.PHONY : all clean deb doc 

all: $(OBJS) $(LIBTARGET)
	@ echo "<< installing $(LIBTARGET)>>"
	@ install -m 644 -p $(LIBTARGET) $(DQM_LIB_PATH)/

clean:
	@ echo "<< cleaning all monitor system >>"
	@ rm -rf $(TMPDIR) $(LIBDIR)
	@ rm -rf *~ */*~ */*/*~ \#*\#* */\#*\#*

$(OBJS): $(TMPDIR)/%.o : $(SRCDIR)/%.cc 
	@ echo '<< compiling $@ >>'
	@ $(CXX) -c $(CFLAGS) $< -o  $@

$(LIBTARGET) : $(OBJS)
	@ echo "<< creating $@ >>"
	@ if [ ! -d $(LIBDIR) ];then $(MKDIR) $(LIBDIR); fi
	@ $(CXX) -shared $(CFLAGS) $^ -o $@ $(LIBS)
	@ echo "<< compiling succeede!! >>"
	@ $(RM) *~ */*~ */*/*~ */*/*/*~ */*/*/*/*~ \#*\#* */\#*\#*

