MODULE_NAME = cprfin-jig
MAJOR_ID = 0
TARGET_CARD_NUMBER = 0

CC := gcc412

obj-m := cprfin-jig.o
KERNEL_VERSION = $(shell uname -r)
PWD = $(shell pwd)

default:
	${MAKE} -C /lib/modules/${KERNEL_VERSION}/build SUBDIRS=${PWD} V=1 modules 

clean:
	rm -f *.o *.ko *.mod.c *.mod.o .*.cmd
	rm -rf .tmp_versions
