CC = gcc
CXX = g++

CFLAGS   = -g -O3 -Wall -fPIC -D_REENTRANT

OBJS = ttrxlib.o bootrx.o stat4r.o

LIB  = libttrx.so

EXTDIR=$(BELLE2_EXTERNALS_DIR)
EXTSRCDIR=$(EXTDIR)/src
EXTBUILDDIR=$(EXTDIR)/build/$(BELLE2_EXTERNALS_SUBDIR)
EXTINCDIR=$(EXTDIR)/include
EXTLIBDIR=$(EXTDIR)/lib/$(BELLE2_EXTERNALS_SUBDIR)
EXTBINDIR=$(EXTDIR)/bin/$(BELLE2_EXTERNALS_SUBDIR)

all: $(LIB)
	@ mkdir -p $(EXTINCDIR)/ttrx/
	@ install -m 644 -p *.h $(EXTINCDIR)/ttrx/
#	@ install -m 644 -p $(LIB) $(EXTLIBDIR)/
	@ if [ -e $(EXTLIBDIR) ];\
	  then \
	  install -m 644 -p $(LIB) $(EXTLIBDIR)/; \
	  else \
	  install -m 644 -p $(LIB) $(EXTDIR)/$(BELLE2_ARCH)/$(BELLE2_EXTERNALS_OPTION)/bin; \
	  fi


clean:
	@ echo "<< clearing directry >>"
	@ rm -rf $(OBJS) $(LIB) *~ .*~ \#*~

$(LIB): $(OBJS)
	@ echo "<< linking shared library $@ >>"
	@ $(CC) -shared $(CFLAGS) $^ -o $@

$(OBJS): %.o : %.c 
	@ echo "<< compiling $@ >>"
	@ $(CC) -c $(CFLAGS) $^ -o $@

