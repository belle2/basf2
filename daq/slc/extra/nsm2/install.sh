nsmdir=${BELLE2_LOCAL_DIR}/daq/slc/extra/nsm2
cd ${nsmdir}
make
install -m 644 ${nsmdir}/corelib/libnsm2_corelib.{a,so} b2lib/libnsm2_b2lib.{a,so} ${BELLE2_EXTERNALS_DIR}/${BELLE2_SUBDIR}/lib/
install -m 755 ${nsmdir}/daemon/{nsmd2,nsminfo2} ${BELLE2_EXTERNALS_DIR}/${BELLE2_SUBDIR}/bin
mkdir -p ${BELLE2_EXTERNALS_DIR}/include/nsm2/
install -m 644 ${nsmdir}/corelib/*.h b2lib/*.h ${BELLE2_EXTERNALS_DIR}/include/nsm2/
make clean
cd -
