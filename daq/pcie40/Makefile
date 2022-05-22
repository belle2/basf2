#export LD_LIBRARY_PATH = $(shell echo $$LD_LIBRARY_PATH):./Pcie40Software/Pcie40Libraries/lib:./Pcie40Software/Pcie40DriverLibraries/pcie40driver_ecs)

SHELL=/bin/bash
CC=g++ -fPIC -g -O3 -Wall -DNOBASF2 -std=c++11
HERE :=$(strip $(realpath $(dir $(lastword $(MAKEFILE_LIST)))))
TOP :=$(realpath $(HERE))/Pcie40Software
TEMP_LOCAL_DIR=$(PWD)/../../
INCDIR = ./include

ZMQINC ?= ${BELLE2_EXTERNALS_DIR}/include
ZMQLIB ?= ${BELLE2_EXTERNALS_DIR}/${BELLE2_ARCH}/common/lib/libzmq.a

LIBFLAG = -lrt -lpthread#-l${LOCALLIB}
DES_SER_PCIE40_MAIN_CXXFLAGS =$(CFLAGS) -I$(TOP)/Pcie40Applications -I$(TOP)/Pcie40Driver -I$(TOP)/Pcie40Libraries -I$(TOP)/Pcie40DriverLibraries -I${INCDIR} -I$(ZMQINC)
DES_SER_PCIE40_MAIN_INSTALL =$(PREFIX)/bin
DES_SER_PCIE40_MAIN_LDFLAGS = -L$(TOP)/build/Pcie40Libraries/ -L$(TOP)/build/Pcie40DriverLibraries/ -lpcie40driver -lpcie40


TARGET1=des_ser_PCIe40_main

all: clean cleanlink link ${TARGET1} cleanlink

.PHONY: link
link:
	@ echo "<< making symlink $@ >>"
	mkdir -p include/rawdata/dataobjects ; \
	ln -s ${TEMP_LOCAL_DIR}/rawdata/dataobjects/include/*.h ./include/rawdata/dataobjects/ ; \
	ln -s ${TEMP_LOCAL_DIR}/rawdata/include/*.h ./include/rawdata/ ; \

.PHONY:	clean
clean:
	@ echo "<< clearing directry >>"
	@ rm -f ${TARGET1} *~ .*~ \#*

.PHONY:	cleanlink
cleanlink:
	@ echo "<< clearing linked header files >>"
	@ rm -rf $(INCDIR)/rawdata 

${TARGET1}: src/${TARGET1}.cc
	source $(TOP)/Scripts/setup.sh; \
	$(CC) $(DES_SER_PCIE40_MAIN_CXXFLAGS) ${LIBFLAG} -L./ $(DES_SER_PCIE40_MAIN_LDFLAGS) $^ -o $@ ${ZMQLIB}
