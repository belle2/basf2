CXX = g++
INCDIR = ./include
CFLAGS   = -g -O3 -Wall -fPIC -I ${INCDIR} -DNOBASF2
#LOCALLIB  = CPRROPCDAQ
LIBFLAG = -lrt -lpthread#-l${LOCALLIB}

TEMP_LOCAL_DIR=$(PWD)/../../

RAWDATAPATH=${TEMP_LOCAL_DIR}/rawdata/src:${TEMP_LOCAL_DIR}/rawdata/dataobjects/src
DAQPATH=${TEMP_LOCAL_DIR}/daq/dataobjects/src:${TEMP_LOCAL_DIR}/daq/rawdata/src
SLCPATH=${TEMP_LOCAL_DIR}/daq/slc/readout/src:${TEMP_LOCAL_DIR}/daq/slc/system/src:${TEMP_LOCAL_DIR}/daq/slc/base/src
VPATH = ${RAWDATAPATH}:${DAQPATH}:${SLCPATH}

RAWDATAOBJS = RawDataBlockFormat.o RawCOPPERFormat.o RawCOPPERFormat_v2.o PreRawCOPPERFormat_v2.o PostRawCOPPERFormat_v2.o RawHeader_v2.o RawTrailer_v2.o RawTLUFormat.o RawFTSWFormat.o RawFTSWFormat_latest.o CRCCalculator.o 
DAQOBJS = DesSer.o DesSerPrePC.o DesSerCOPPER.o CprErrorMessage.o SendHeader.o SendTrailer.o
SLCOBJS = RunInfoBuffer.o SharedMemory.o MMutex.o MCond.o LogFile.o Mutex.o Date.o ConfigFile.o StringUtil.o
OBJS = ${RAWDATAOBJS} ${DAQOBJS} ${SLCOBJS}


LIBDIR=$(BELLE2_LOCAL_DIR)/lib

TARGET1=des_ser_ROPC_main
TARGET2=des_ser_COPPER_main

all: clean link ${TARGET1} ${TARGET2} cleanobj

ropc: clean_ropc link ${TARGET1} cleanobj

copper: clean_copper link ${TARGET2} cleanobj
	@ echo "!!!!!!! Martin added overrides to reduce cppcheck warnings (https://stash.desy.de/projects/B2/repos/software/pull-requests/2825/overview), which leads to failure of compiling on COPPER. To compile on a COPPER, please run the following commands to remove override in rawdata classes"
	@ echo "!!!!!!! cd rawdata/dataobjects/include; sed -i -e 's/ override;/;/g' *.h"


${TARGET1}: ${TARGET1}.cc ${OBJS}
	$(CXX) $(CFLAGS) ${LIBFLAG} -L./ $^ -o $@ 

${TARGET2}: ${TARGET2}.cc ${OBJS}
	$(CXX) $(CFLAGS) ${LIBFLAG} -L./ $^ -o $@ 

$(OBJS): %.o : %.cc
	@ echo "<< compiling $@ >>"
	@ $(CXX) -c $(CFLAGS) $<

#${LOCALLIB}: $(OBJS)
#	@ echo "<< linking shared library $@ >>"
#	@ $(CXX) -shared $(CFLAGS) $^ -o lib$@.so

#.c.o:
#	$(CXX) $(CFLAGS) -c ${INC} $<

.PHONY: link
link:
	@ echo "<< making smylink $@ >>"
	@if [ ! -d ${INCDIR} ]; then \
	mkdir -p include/daq/slc/system ; \
	mkdir -p include/daq/slc/readout ; \
	mkdir -p include/daq/slc/base ; \
	mkdir -p include/daq/rawdata ; \
	mkdir -p include/rawdata/dataobjects ; \
	mkdir -p include/daq/rawdata/modules ; \
	mkdir -p include/daq/dataobjects ; \
	ln -s ${TEMP_LOCAL_DIR}/rawdata/dataobjects/include/*.h ./include/rawdata/dataobjects/ ; \
	ln -s ${TEMP_LOCAL_DIR}/rawdata/include/*.h ./include/rawdata/ ; \
	ln -s ${TEMP_LOCAL_DIR}/daq/rawdata/include/*.h ./include/daq/rawdata/ ; \
	ln -s ${TEMP_LOCAL_DIR}/daq/rawdata/modules/include/*.h ./include/daq/rawdata/modules/ ; \
	ln -s ${TEMP_LOCAL_DIR}/daq/dataobjects/include/*.h ./include/daq/dataobjects/ ; \
	ln -s ${TEMP_LOCAL_DIR}/daq/slc/system/include/*.h ./include/daq/slc/system/ ; \
	ln -s ${TEMP_LOCAL_DIR}/daq/slc/readout/include/*.h ./include/daq/slc/readout/ ; \
	ln -s ${TEMP_LOCAL_DIR}/daq/slc/base/include/*.h ./include/daq/slc/base/ ; \
	fi

.PHONY:	clean
clean:
	@ echo "<< clearing directry >>"
	@ rm -rf ${TARGET1} ${TARGET2} $(OBJS) lib${LOCALLIB}.so *~ .*~ \#*~
	@ rm -rf ${INCDIR}

.PHONY:	clean_ropc
clean_ropc:
	@ echo "<< clearing directry >>"
	@ rm -rf ${TARGET1} $(OBJS) lib${LOCALLIB}.so *~ .*~ \#*~
	@ rm -rf ${INCDIR}

.PHONY:	clean_copper
clean_copper:
	@ echo "<< clearing directry >>"
	@ rm -rf ${TARGET2} $(OBJS) lib${LOCALLIB}.so *~ .*~ \#*~
	@ rm -rf ${INCDIR}

.PHONY:	cleanobj
cleanobj:
	@ echo "<< clearing object files >>"
	@ rm -rf $(OBJS) 
