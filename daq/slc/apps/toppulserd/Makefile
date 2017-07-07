all: copyinc apps/toppulserd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/toppulserd
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/toppulserd/include/* $(INCDIR)/apps/toppulserd

apps/toppulserd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
