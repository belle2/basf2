# Makefile for FTSW programs

CC = gcc

SRCS   = $(wildcard *.c)
OBJS   = $(SRCS:%.c=%.o)

HDRS = ftsw.h
LVME = -lvme
LIBS = $(OBJS) -lm $(LVME)

CFLAGS = -DUSE_LINUX_VME_UNIVERSE=1
LDFLAGS = -g

LIBTARGET  = libstatft.so

.c.o:
	$(CC) -c -g $(CFLAGS) -o $@ $<

all: $(LIBTARGET)
	mkdir -p $(BELLE2_DAQ_SLC)/include/ftprogs
	install -m 644 -p ftsw.h $(BELLE2_DAQ_SLC)/include/ftprogs
	install -m 644 -p ftstat.h $(BELLE2_DAQ_SLC)/include/ftprogs
	install -m 644 -p $(LIBTARGET) $(BELLE2_DAQ_SLC)/lib

clean:
	rm -f *.o *~ *.bak $(LIBTARGET)

$(LIBTARGET): $(OBJS) $(HDRS)
	$(CC) -shared $(LDFLAGS) $(LIBS) -o $@

