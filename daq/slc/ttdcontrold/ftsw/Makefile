CC = gcc
#CC = gcc44

CFLAGS   = -g -O3 -Wall -fPIC -D_REENTRANT -DUSE_LINUX_VME_UNIVERSE=1 -I.

OBJS = ftswlib.o ftsw_bootft.o

APPS = bootft regft trig_loop

LIBS = libFTSW.a

all: $(LIBS) $(APPS)

clean:
	@ echo "<< clearing directry >>"
	@ rm -rf $(OBJS) $(LIBS) $(APPS) *~ .*~ \#*~

$(LIBS): $(OBJS)
	@ ar rcsv $@ $^

$(OBJS): %.o : %.c
	@ echo "<< compiling $@ >>"
	@ $(CC) -c $(CFLAGS) $^ -o $@

$(APPS): % : %.c
	@ echo "<< compiling $@ >>"
	@ $(CC) $(CFLAGS) $^ -o $@ -L. -lFTSW -lm -lvme