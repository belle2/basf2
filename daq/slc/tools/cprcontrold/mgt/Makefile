CC = gcc
CXX = g++

CFLAGS   = -g -O3 -Wall -fPIC -D_REENTRANT

OBJS = find_pci_path.o mgt.o mgt_callback.o

LIB  = libMGT.so

all: $(OBJS)

clean:
	@ echo "<< clearing directry >>"
	@ rm -rf $(OBJS) $(LIB) *~ .*~ \#*~

$(LIB): $(OBJS)
	@ echo "<< linking shared library $@ >>"
	@ $(CC) -shared $(CFLAGS) $^ -o $@

$(OBJS): %.o : %.c 
	@ echo "<< compiling $@ >>"
	@ $(CC) -c $(CFLAGS) $^ -o $@

