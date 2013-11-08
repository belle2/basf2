MODULE_NAME = cprfin_fngeneric

#CC := gcc412
CC := /usr/bin/gcc
obj-m := cprfin_fngeneric.o
#KERNEL_VERSION = $(shell uname -r)
KERNEL_VERSION = 2.6.18
PWD = $(shell pwd)

default:
	${MAKE} -C /lib/modules/${KERNEL_VERSION}/build SUBDIRS=${PWD} V=1 modules CC=${CC}

clean:
	rm -f *.o *.ko *.mod.c *.mod.o .*.cmd
	rm -rf .tmp_versions
