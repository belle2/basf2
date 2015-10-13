#!/bin/bash

export EXTDIR=${BELLE2_EXTERNALS_DIR}
export EXTSRCDIR=${EXTDIR}/src
export EXTBUILDDIR=${EXTDIR}/build/${BELLE2_EXTERNALS_SUBDIR}
export EXTINCDIR=${EXTDIR}/include
EXTLIBDIR=${EXTDIR}/lib/${BELLE2_EXTERNALS_SUBDIR}
EXTBINDIR=${EXTDIR}/bin/${BELLE2_EXTERNALS_SUBDIR}
if [ -e ${EXTLIBDIR} ]; then
export ${EXTLIBDIR}
export ${EXTBINDIR}
else
export EXTLIBDIR=${EXTDIR}/build/${BELLE2_EXTERNALS_SUBDIR}
export EXTBINDIR=${EXTDIR}/${BELLE2_ARCH}/${BELLE2_EXTERNALS_OPTION}/bin
fi

SLC_DIR=${BELLE2_LOCAL_DIR}/daq/slc/extra/nsm2/

install -m 644 -p ${SLC_DIR}/daemon/*.* $BELLE2_EXTERNALS_DIR/src/nsm2/daemon
install -m 644 -p ${SLC_DIR}/daemon/Makefile $BELLE2_EXTERNALS_DIR/src/nsm2/daemon
install -m 644 -p ${SLC_DIR}/corelib/Makefile $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
install -m 644 -p ${SLC_DIR}/corelib/*.c $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
install -m 644 -p ${SLC_DIR}/corelib/nsm2.h $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
install -m 644 -p ${SLC_DIR}/corelib/nsmlib2.h $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
install -m 644 -p ${SLC_DIR}/corelib/nsmparse.h $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
install -m 644 -p ${SLC_DIR}/b2lib/*.* $BELLE2_EXTERNALS_DIR/src/nsm2/b2lib
install -m 644 -p ${SLC_DIR}/corelib/*.h $BELLE2_EXTERNALS_DIR/include/nsm2/
install -m 644 -p ${SLC_DIR}/b2lib/belle2nsm.h $BELLE2_EXTERNALS_DIR/include/nsm2/b2lib
#make -C $BELLE2_EXTERNALS_DIR/src/nsm2/corelib clean
#make -C $BELLE2_EXTERNALS_DIR/src/nsm2/b2lib clean
#make -C $BELLE2_EXTERNALS_DIR/src/nsm2/daemon clean
#make -C $BELLE2_EXTERNALS_DIR/src/nsm2 clean
#rm -f $EXTLIBDIR/nsmd2 $EXTLIBDIR/nsminfo2
make -C $BELLE2_EXTERNALS_DIR/src/nsm2 
make -C $BELLE2_EXTERNALS_DIR/src/nsm2 install
