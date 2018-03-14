all: copyinc hvcontrol/bklm

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/
LIBDIR=$(BELLE2_DAQ_SLC)/lib

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/bklm
	@ install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/bklm/include/* $(INCDIR)/hvcontrol/bklm
	@ mkdir -p $(INCDIR)$/hvcontrol/bklm/caen
	@ install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/bklm/caen/include/* $(INCDIR)/hvcontrol/bklm/caen
	@ install -m 755 -p $(BELLE2_DAQ_SLC)/hvcontrol/bklm/caen/lib/* $(LIBDIR) 
	#@ install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/bklm/CAENHVWrapper-5.56/include/* $(INCDIR)/hvcontrol/bklm/caen
	#@ install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/bklm/CAENHVWrapper-5.56/HVWrapperDemo/*h $(INCDIR)/hvcontrol/bklm/caen
	#@ install -m 755 -p $(BELLE2_DAQ_SLC)/hvcontrol/bklm/CAENHVWrapper-5.56/lib/x64/*.so* $(LIBDIR)

hvcontrol/bklm: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
