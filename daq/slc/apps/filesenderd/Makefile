all: copyinc apps/filesenderd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/filesenderd
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/filesenderd/include/* $(INCDIR)/apps/filesenderd

apps/filesenderd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
