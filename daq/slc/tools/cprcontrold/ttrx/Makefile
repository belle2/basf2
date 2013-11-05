# Makefile for TT-RX (v3/v4) programs

EXE1 = bootrx
EXE2 = dumprx
EXE3 = regrx
EXE4 = trigrx
EXE5 = #fiforx
EXE6 = #tchkrx

TARGETS = $(EXE1) $(EXE2) $(EXE3) $(EXE4) $(EXE5) $(EXE6)

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

LIB  = libttrx.a
HDRS = ttrx.h
OBJS = ttrxlib.o
LIBS = $(OBJS) -lm

prefix = /belle/local/depot/ttrx

.c.o:
	$(CC) -c -g $(CFLAGS) $(INCS) -o $@ $<

all: $(TARGETS) $(LIB)

install:
	install -d $(prefix)/bin
	install -d $(prefix)/lib
	install -d $(prefix)/include/ttd
	install $(TARGETS) $(prefix)/bin
	install $(LIB)     $(prefix)/lib
	install $(HDRS)    $(prefix)/include/ttd

clean:
	rm -f *.o *.a *~ *.bak $(TARGETS)

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


$(LIB): $(OBJS)
	$(AR) rs $(LIB) $(OBJS)
