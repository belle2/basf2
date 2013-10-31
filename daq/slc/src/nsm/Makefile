CXX     = g++
#CXX     = g++44
CP      = cp
MAKE    = make
MKDIR   = mkdir
AR      = ar rcsv
RM      = rm -f

NSM2_PATH = $(B2SLC_PATH)/nsm2

HOMEDIR  = .
SRCDIR   = $(HOMEDIR)/src
INCDIR   = $(HOMEDIR)/include
BINDIR   = $(HOMEDIR)/bin
TMPDIR   = $(HOMEDIR)/tmp
LIBDIR   = $(HOMEDIR)/lib

LIBTARGET  = $(LIBDIR)/libdaq_slc_nsm.so

SRCS     = $(wildcard $(SRCDIR)/*.cc)
INCS     = $(wildcard $(INCDIR)/*.h)
OBJS     = $(SRCS:$(SRCDIR)/%.cc=$(TMPDIR)/%.o)
DEPS     = $(OBJS:%.o=%.d)

CFLAGS   = -g -fPIC -O2 -Wall
CFLAGS  += -I$(INCDIR) 
CFLAGS  += -I$(B2SLC_PATH)/include 
CFLAGS  += -I$(NSM2_PATH)/corelib -I$(NSM2_PATH)/b2lib

.PHONY : all clean 

all: $(DEPS) $(OBJS) $(LIBTARGET)

clean:
	@ echo "<< cleaning all monitor system >>"
	@ $(RM) -rf $(TMPDIR)/* $(LIBDIR)
	@ $(RM) -rf $(DOCDIR)/html $(INCDIR)/.h
	@ $(RM) *~ */*~ */*/*~ \#*\#* */\#*\#*

$(LIBTARGET) : $(OBJS)
	@ echo "<< creating shared librariy $@ >>"
	@ if [ ! -d $(LIBDIR) ];then $(MKDIR) $(LIBDIR); fi
#	@ $(CXX) -shared $(CFLAGS) $(NSM_OBJS) $^ -o $@ $(LIBS)
	@ $(CXX) -shared $(CFLAGS) $^ -o $@ $(LIBS)
	@ echo "<< compiling succeede!! >>"
	@ $(RM) *~ */*~ */*/*~ */*/*/*~ */*/*/*/*~ \#*\#* */\#*\#*

$(OBJS) : $(TMPDIR)/%.o : $(SRCDIR)/%.cc $(INCDIR)/%.h
	@ echo "<< compiling $@ >>"
	@ $(CXX) -c $(CFLAGS) $< -o $@ 

$(DEPS) : $(TMPDIR)/%.d: $(SRCDIR)/%.cc $(INCDIR)/%.h
	@ if [ ! -d $(TMPDIR) ];then $(MKDIR) $(TMPDIR); fi
	@ echo "<< making dependency $@ >>"
	@ $(CXX) $(CFLAGS) -MM -MT $(basename $@).o $< -o $@

-include $(DEPS)
