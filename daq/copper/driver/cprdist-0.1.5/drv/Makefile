MODULE_NAME = copper
MAJOR_ID = 0
TARGET_CARD_NUMBER = 0

CC := /usr/bin/gcc
#CC := gcc412
CFLAGS += -O3
obj-m := copper.o
copper-objs := cpr.o evq.o
#KERNEL_VERSION = $(shell uname -r)
KERNEL_VERSION = 2.6.18
PWD = $(shell pwd)

default:
	${MAKE} -C /lib/modules/${KERNEL_VERSION}/build SUBDIRS=${PWD} V=1 modules CC=${CC} 

all:
	${MAKE} -C /lib/modules/${KERNEL_VERSION}/build SUBDIRS=${PWD} V=1 modules CC=${CC}

clean:
	rm -f *.o *.ko *.mod.c *.mod.o .*.cmd
	rm -rf .tmp_versions
 
