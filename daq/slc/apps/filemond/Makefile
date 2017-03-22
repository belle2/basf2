all: copyinc apps/filemond

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/filemond
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/filemond/include/* $(INCDIR)/apps/filemond

apps/filemond: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
