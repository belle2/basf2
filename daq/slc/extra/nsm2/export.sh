#!/bin/bash

export EXTDIR=${BELLE2_EXTERNALS_DIR}
export EXTSRCDIR=${EXTDIR}/src
export EXTBUILDDIR=${EXTDIR}/build/${BELLE2_EXTERNALS_SUBDIR}
export EXTINCDIR=${EXTDIR}/include
export EXTLIBDIR=${EXTDIR}/lib/${BELLE2_EXTERNALS_SUBDIR}
export EXTBINDIR=${EXTDIR}/bin/${BELLE2_EXTERNALS_SUBDIR}

cp corelib/nsm2.h $BELLE2_EXTERNALS_DIR/src/nsm2/daemon
cp corelib/nsmparse.c $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
cp corelib/nsmlib2.c $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
cp corelib/nsmlib2.h $BELLE2_EXTERNALS_DIR/src/nsm2/corelib
cp corelib/nsmlib2.h $BELLE2_EXTERNALS_DIR/include/nsm2/
cp b2lib/belle2nsm.c $BELLE2_EXTERNALS_DIR/src/nsm2/b2lib
cp b2lib/belle2nsm.h $BELLE2_EXTERNALS_DIR/src/nsm2/b2lib
make -C $BELLE2_EXTERNALS_DIR/src/nsm2 
make -C $BELLE2_EXTERNALS_DIR/src/nsm2 install

