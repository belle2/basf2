# Makefile for HSLB software (hsprogs)
#
# VERSION 20160105

#EXE1 = booths
EXE2 = reghs
EXE3 = readhs
EXE4 = tesths
EXE5 = staths
#EXE6 = copperstat
#EXE7 = reghsx
EXE8 = 
EXE9 = 

TARGETS = $(LIB) \
 $(EXE1) $(EXE2) $(EXE3) $(EXE4) $(EXE5) $(EXE6) $(EXE7) $(EXE8) $(EXE9)

SRCL = libhslb.c
SRC1 = $(EXE1).c
SRC2 = $(EXE2).c
SRC3 = $(EXE3).c
SRC4 = $(EXE4).c
SRC5 = $(EXE5).c
SRC6 = $(EXE6).c
SRC7 = $(EXE7).c
SRC8 = $(EXE8).c
SRC9 = $(EXE9).c

OBJL = libhslb.o
OBJ1 = $(EXE1).o
OBJ2 = $(EXE2).o
OBJ3 = $(EXE3).o
OBJ4 = $(EXE4).o
OBJ5 = $(EXE5).o hsreg.o
OBJ6 = $(EXE6).o
OBJ7 = $(EXE7).o
OBJ8 = $(EXE8).o
OBJ9 = $(EXE9).o

HDRS = libhslb.h Makefile
LIB  = libhslb.a
LIBS = -L. -lhslb -lm
LDFLAGS = -g
INCS =

prefix = /usr/local

.c.o:
	$(CC) -c -g $(CFLAGS) $(INCS) -o $@ $<

all: $(TARGETS) $(LIB)

install:
	sudo install -d $(prefix)/bin
	sudo install $(TARGETS) $(prefix)/bin

clean:
	rm -f *.o *~ *.bak $(TARGETS)

$(LIB): $(OBJL) Makefile
	$(AR) rs $(LIB) $(OBJL)

$(EXE1): $(OBJ1) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ1) $(LIBS)

$(EXE2): $(OBJ2) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ2) $(LIBS)

$(EXE3): $(OBJ3) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ3) $(LIBS)

$(EXE4): $(OBJ4) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ4) $(LIBS)

$(EXE5): $(OBJ5) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ5) $(LIBS)

$(EXE6): $(OBJ6) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ6) $(LIBS)

$(EXE7): $(OBJ7) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ7) $(LIBS)

$(EXE8): $(OBJ8) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ8) $(LIBS)

$(EXE9): $(OBJ9) $(LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJ9) $(LIBS)

$(OBJL): $(SRCL) $(HDRS)
$(OBJ1): $(SRC1) $(HDRS)
$(OBJ2): $(SRC2) $(HDRS)
$(OBJ3): $(SRC3) $(HDRS)
$(OBJ4): $(SRC4) $(HDRS)
$(OBJ5): $(SRC5) $(HDRS)
$(OBJ6): $(SRC6) $(HDRS)
$(OBJ7): $(SRC7) $(HDRS)
$(OBJ8): $(SRC8) $(HDRS)
$(OBJ9): $(SRC9) $(HDRS)
