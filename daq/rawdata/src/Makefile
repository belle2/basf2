CC = /usr/bin/gcc
#CC = gcc
CXX = g++
INCDIR = ../include
CFLAGS   = -g -O3 -Wall -fPIC -I ${INCDIR}


INC = -I ${BELLE2_LOCAL_DIR}/include
#INC = -I ${BELLE2_LOCAL_DIR}/rawdata/dataobjects/include

OBJSRCDIR1 = ${BELLE2_LOCAL_DIR}/rawdata/dataobjects/src
OBJSRCDIR2 = ${BELLE2_LOCAL_DIR}/rawdata/src


#OBJS1 = RawCOPPERFormat.o RawCOPPERFormat_latest.o PreRawCOPPERFormat_latest.o PostRawCOPPERFormat_latest.o RawHeader_latest.o RawTrailer_latest.o
#OBJS2 =  CRCCalculator.o 
#OBJS = ${OBJSOURCES}/*.

LIB  = -ldaq -ldaq_dataobjects -ldaq_rawdata -ldataobjects -lrawdata_dataobjects -lrawdata
#LIB  = daq


LIBDIR=$(BELLE2_LOCAL_DIR)/lib/Linux_i686/opt
TARGET=des_ser_prePC_main

#all: $(OBJS)


all: ${TARGET}


#daq_basf2_free : daq_basf2_free.cc ${LIB}
${TARGET}: ${TARGET}.cc 
	$(CXX) $(CFLAGS) ${INC}  -L${LIBDIR} $^ -o $@ ${LIB} 


$(LIB): $(OBJS1) $(OBJS2)
	@ echo "<< linking shared library $@ >>"
	@ $(CXX) -shared $(CFLAGS) $^ -o $@n

$(OBJS1): %.o : ${OBJSRCDIR1}/%.cc
	@ echo "<< making smylink $@ >>"
	@if [ ! -d ${INCDIR} ]; then \
	mkdir -p include/rawdata/dataobjects ; \
	ln -s ${BELLE2_LOCAL_DIR}/rawdata/dataobjects/include/*.h ./include/rawdata/dataobjects/ ; \
	ln -s ${BELLE2_LOCAL_DIR}/rawdata/include/*.h ./include/rawdata/ ; \
	fi
	@ echo "<< compiling $@ >>"
	@ $(CXX) -c $(CFLAGS) $<

$(OBJS2): %.o : ${OBJSRCDIR2}/%.cc
	@ echo "<< making smylink $@ >>"
	@if [ ! -d ${INCDIR} ]; then \
	mkdir -p include/rawdata/dataobjects ; \
	ln -s ${BELLE2_LOCAL_DIR}/rawdata/dataobjects/include/*.h ./include/rawdata/dataobjects/ ; \
	ln -s ${BELLE2_LOCAL_DIR}/rawdata/include/*.h ./include/rawdata/ ; \
	fi
	@ echo "<< compiling $@ >>"
	@ $(CXX) -c $(CFLAGS) $<

#.c.o:
#	$(CXX) $(CFLAGS) -c ${INC} $<


.PHONY:	clean
clean:
	@ echo "<< clearing directry >>"
	@ rm -rf ${TARGET} *.o
#	@ rm -rf $(OBJS1) ${OBJS2} $(LIB) *~ .*~ \#*~
#	@ rm -rf ${INCDIR}