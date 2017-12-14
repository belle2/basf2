# Makefile for TT-IO programs

CC = gcc

EXE1 = bootft
EXE2 = regft
EXE3 = #dumpft
EXE4 = #statft
EXE5 = #ftor_sfp
EXE6 = #utimeft
EXE7 = trigft
EXE8 = 
EXE9 = 

TARGETS = $(EXE1) $(EXE2) $(EXE3) $(EXE4) $(EXE5) $(EXE6) $(EXE7) $(EXE8) $(EXE9)

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

HDRS = ftsw.h
OBJS = ftswlib.o
LIBS = $(OBJS) -lm -lvme

prefix = /belle/local/depot/ttd

CFLAGS = -DUSE_LINUX_VME_UNIVERSE=1
LDFLAGS = -g

.c.o:
	$(CC) -c -g $(CFLAGS) $(INCS) -o $@ $<

all: $(TARGETS)

install:
	install $(TARGETS) $(prefix)/bin

clean:
	rm -f *.o *~ *.bak $(TARGETS)

$(EXE1): $(OBJ1) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ1) $(LIBS)

$(EXE2): $(OBJ2) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ2) $(LIBS)

$(EXE3): $(OBJ3) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ3) $(LIBS)

$(EXE4): $(OBJ4) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ4) $(LIBS)

$(EXE5): $(OBJ5) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ5) $(LIBS)

$(EXE6): $(OBJ6) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ6) $(LIBS)

$(EXE7): $(OBJ7) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ7) $(LIBS)

$(EXE8): $(OBJ8) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ8) $(LIBS)

$(EXE9): $(OBJ9) $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) -o $@ $(OBJ9) $(LIBS)

$(OBJ1): $(SRC1) $(HDRS)
$(OBJ2): $(SRC2) $(HDRS)
$(OBJ3): $(SRC3) $(HDRS)
$(OBJ4): $(SRC4) $(HDRS)
$(OBJ5): $(SRC5) $(HDRS)
$(OBJ6): $(SRC6) $(HDRS)
$(OBJ7): $(SRC7) $(HDRS)
$(OBJ8): $(SRC8) $(HDRS)
$(OBJ9): $(SRC9) $(HDRS)
