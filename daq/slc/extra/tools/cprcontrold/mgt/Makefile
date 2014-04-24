CC = gcc
CXX = g++

CFLAGS   = -g -O3 -Wall -fPIC -D_REENTRANT

OBJS = find_pci_path.o mgt.o 

LIB  = libmgt.so

EXTDIR=$(BELLE2_EXTERNALS_DIR)
EXTSRCDIR=$(EXTDIR)/src
EXTBUILDDIR=$(EXTDIR)/build/$(BELLE2_EXTERNALS_SUBDIR)
EXTINCDIR=$(EXTDIR)/include
EXTLIBDIR=$(EXTDIR)/lib/$(BELLE2_EXTERNALS_SUBDIR)
EXTBINDIR=$(EXTDIR)/bin/$(BELLE2_EXTERNALS_SUBDIR)

#all: $(OBJS)
all: $(LIB)
	@ mkdir -p $(EXTINCDIR)/mgt/
	@ install -m 644 -p *.h $(EXTINCDIR)/mgt/
	@ install -m 644 -p $(LIB) $(EXTLIBDIR)/

clean:
	@ echo "<< clearing directry >>"
	@ rm -rf $(OBJS) $(LIB) *~ .*~ \#*~

$(LIB): $(OBJS)
	@ echo "<< linking shared library $@ >>"
	@ $(CC) -shared $(CFLAGS) $^ -o $@

$(OBJS): %.o : %.c 
	@ echo "<< compiling $@ >>"
	@ $(CC) -c $(CFLAGS) $^ -o $@

