# Makefile for TT-RX (v3/v4) programs
#
# VERSION 2010060400

#EXE1 = booths
EXE2 = reghs
#EXE3 = readhs
#EXE4 = tesths
#EXE5 = staths
#EXE6 = copperstat
EXE7 = 
EXE8 = 
EXE9 = 

TARGETS = $(EXE1) $(EXE2) $(EXE3) $(EXE4) $(EXE5) $(EXE6) $(EXE7) $(EXE8) \
          $(EXE9)

OBJ1 = $(EXE1).o
SRC1 = $(EXE1).c
OBJ2 = $(EXE2).o
SRC2 = $(EXE2).c
OBJ3 = $(EXE3).o
SRC3 = $(EXE3).c
OBJ4 = $(EXE4).o
SRC4 = $(EXE4).c
OBJ5 = $(EXE5).o
SRC5 = $(EXE5).c
OBJ6 = $(EXE6).o
SRC6 = $(EXE6).c
OBJ7 = $(EXE7).o
SRC7 = $(EXE7).c
OBJ8 = $(EXE8).o
SRC8 = $(EXE8).c
OBJ9 = $(EXE9).o
SRC9 = $(EXE9).c

LIB  = 
HDRS = find_pci_path.h libhslb.h
OBJS = find_pci_path.o libhslb.o
LIBS = $(OBJS) -lm
LDFLAGS = -g
INCS = -I/belle/local/include

prefix = /belle/local/depot/mgt

.c.o:
	$(CC) -c -g $(CFLAGS) $(INCS) -o $@ $<

all: $(TARGETS) $(LIB)

install:
	install -d $(prefix)/bin
	install $(TARGETS) $(prefix)/bin

clean:
	rm -f *.o *~ *.bak $(TARGETS)

$(EXE1): $(OBJ1) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ1) $(LIBS)

$(EXE2): $(OBJ2) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ2) $(LIBS)

$(EXE3): $(OBJ3) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ3) $(LIBS)

$(EXE4): $(OBJ4) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ4) $(LIBS)

$(EXE5): $(OBJ5) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ5) $(LIBS)

$(EXE6): $(OBJ6) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ6) $(LIBS)

$(EXE7): $(OBJ7) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ7) $(LIBS)

$(EXE8): $(OBJ8) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ8) $(LIBS)

$(EXE9): $(OBJ9) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ9) $(LIBS)

#	$(CC) $(LDFLAGS) -o $@ $(OBJ9) $(LIBS) -L/belle/local/lib -lnsmlib


$(LIB): $(OBJS)
	$(AR) rs $(LIB) $(OBJS)
