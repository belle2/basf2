USER_CLASS=ArichHV
DQM_LIB_PATH=/home/tkonno/belle2/release/daq/slc/data/dqm/lib
CXX     = g++

HOMEDIR  = .
SRCDIR   = $(HOMEDIR)/src
INCDIR   = $(HOMEDIR)/include
BINDIR   = $(HOMEDIR)/bin
TMPDIR   = $(HOMEDIR)/tmp
LIBDIR   = $(HOMEDIR)/lib

BELLE2_LIB = $(BELLE2_LOCAL_DIR)/lib/$(BELLE2_SUBDIR)
BELLE2_INC = $(BELLE2_LOCAL_DIR)/include
BELLE2EX_LIB = $(BELLE2_EXTERNALS_DIR)/lib/$(BELLE2_SUBDIR)
BELLE2EX_INC = $(BELLE2_EXTERNALS_DIR)/include

SRCS   = $(SRCDIR)/$(USER_CLASS)Monitor.cc $(SRCDIR)/lib$(USER_CLASS)Monitor.cc 
OBJS   = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)

CFLAGS   = -g -fPIC -O2 -Wall
CFLAGS  += -I$(INCDIR)
CFLAGS  += -I$(BELLE2_INC)
CFLAGS  += -I$(BELLE2EX_INC)

LIBTARGET  = $(LIBDIR)/libdaq_slc_dqm_$(USER_CLASS).so

.PHONY : all clean deb doc 

all: $(OBJS) $(LIBTARGET)
	@ echo "<< installing $(LIBTARGET)>>"
	@ mkdir -p $(DQM_LIB_PATH)/
	@ install -m 644 -p $(LIBTARGET) $(DQM_LIB_PATH)/

clean:
	@ echo "<< cleaning all monitor system >>"
	@ rm -rf $(TMPDIR) $(LIBDIR)
	@ rm -rf *~ */*~ */*/*~ \#*\#* */\#*\#*

$(OBJS): $(TMPDIR)/%.o : $(SRCDIR)/%.cc 
	@ if [ ! -d $(TMPDIR) ];then mkdir $(TMPDIR); fi
	@ echo '<< compiling $@ >>'
	@ $(CXX) -c $(CFLAGS) $< -o  $@

$(LIBTARGET) : $(OBJS)
	@ echo "<< creating $@ >>"
	@ if [ ! -d $(LIBDIR) ];then mkdir $(LIBDIR); fi
	@ $(CXX) -shared $(CFLAGS) $^ -o $@ $(LIBS)
	@ echo "<< compiling succeede!! >>"
	@ $(RM) *~ */*~ */*/*~ */*/*/*~ */*/*/*/*~ \#*\#* */\#*\#*

