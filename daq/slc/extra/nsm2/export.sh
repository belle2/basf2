#!/bin/bash

export EXTDIR=${BELLE2_EXTERNALS_DIR}
export EXTSRCDIR=${EXTDIR}/src
export EXTBUILDDIR=${EXTDIR}/build/${BELLE2_EXTERNALS_SUBDIR}
export EXTINCDIR=${EXTDIR}/include
export EXTLIBDIR=${EXTDIR}/lib/${BELLE2_EXTERNALS_SUBDIR}
export EXTBINDIR=${EXTDIR}/bin/${BELLE2_EXTERNALS_SUBDIR}
SLC_DIR=${BELLE2_LOCAL_DIR}/daq/slc/extra/nsm2/

cp ${SLC_DIR}/daemon/*.* $BELLE2_EXTERNALS_DIR/src/nsm2/daemon
cp ${SLC_DIR}/corelib/Makefile $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
cp ${SLC_DIR}/corelib/*.c $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
cp ${SLC_DIR}/corelib/nsmlib2.h $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
cp ${SLC_DIR}/corelib/nsmparse.h $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
cp ${SLC_DIR}/b2lib/*.* $BELLE2_EXTERNALS_DIR/src/nsm2/b2lib
cp ${SLC_DIR}/corelib/*.h $BELLE2_EXTERNALS_DIR/include/nsm2/
cp ${SLC_DIR}/b2lib/belle2nsm.h $BELLE2_EXTERNALS_DIR/include/nsm2/b2lib
make -C $BELLE2_EXTERNALS_DIR/src/nsm2 clean
make -C $BELLE2_EXTERNALS_DIR/src/nsm2 
make -C $BELLE2_EXTERNALS_DIR/src/nsm2 install

