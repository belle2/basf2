CC = gcc
CXX = g++

CFLAGS   = -g -O3 -Wall -fPIC -D_REENTRANT

OBJS = ttrxlib.o bootrx.o stat4r.o

LIB  = libttrx.so

INCDIR=$(BELLE2_DAQ_SLC)/include
LIBDIR=$(BELLE2_DAQ_SLC)/lib

all: $(OBJS) $(LIB)
	@ mkdir -p $(INCDIR)/ttrx/
	@ install -m 644 -p *.h $(INCDIR)/ttrx/
	@ install -m 644 -p $(LIB) $(LIBDIR)/

clean:
	@ echo "<< clearing directry >>"
	@ rm -rf $(OBJS) $(LIB) *~ .*~ \#*~

$(LIB): $(OBJS)
	@ echo "<< linking shared library $@ >>"
	@ $(CC) -shared $(CFLAGS) $^ -o $@

$(OBJS): %.o : %.c 
	@ echo "<< compiling $@ >>"
	@ $(CC) -c $(CFLAGS) $^ -o $@

