# Makefile for FTSW programs

CC = gcc -fPIC

SRCS   = $(wildcard *.c)
OBJS   = $(SRCS:%.c=%.o)

HDRS = ftsw.h
LVME = -lvme
LIBS = $(OBJS) -lm $(LVME) 

CFLAGS = -DUSE_LINUX_VME_UNIVERSE=1
CFLAGS += -DSTATFT_NSM
LDFLAGS = -g -DSTATFT_NSM

LIBTARGET  = libftprogs2.so

.c.o:
	$(CC) -c -g $(CFLAGS) -o $@ $<

all: $(LIBTARGET)
	mkdir -p $(BELLE2_DAQ_SLC)/include/ftprogs2
	install -m 644 -p ft*.h $(BELLE2_DAQ_SLC)/include/ftprogs2
	install -m 644 -p pocket*.h $(BELLE2_DAQ_SLC)/include/ftprogs2
	install -m 644 -p $(LIBTARGET) $(BELLE2_DAQ_SLC)/lib

clean:
	rm -f *.o *~ *.bak $(LIBTARGET)

$(LIBTARGET): $(OBJS) $(HDRS)
	$(CC) -shared $(LDFLAGS) $(LIBS) -o $@

