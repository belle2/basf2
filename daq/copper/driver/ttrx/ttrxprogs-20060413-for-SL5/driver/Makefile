MODS = ttrx.ko ttrx_fifo.ko
OBJS = ttrx.o ttrx_fifo.o

all: ${MODS}

CC := gcc412

#CFLAGS += -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing  
#CFLAGS += -fno-common -fomit-frame-pointer -pipe -mpreferred-stack-boundary=2 
#CFLAGS += -march=i686
CFLAGS += -mhard-float

KERNEL_VERSION = $(shell uname -r)
PWD = $(shell pwd)

obj-m := ${OBJS}

%.ko: %.c
	make -C /lib/modules/${KERNEL_VERSION}/build SUBDIRS=${PWD} V=1 modules

clean:
	rm -f *.o *.ko *.mod.c *.mod.o .*.cmd
	rm -rf .tmp_versions
