all: copyinc apps/runrecordd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/runrecordd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/runrecordd/include/* $(INCDIR)/apps/runrecordd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/runrecordd/include/* $(INCDIR)/apps/runrecordd

apps/runrecordd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
