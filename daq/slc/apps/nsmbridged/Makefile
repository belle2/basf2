all: copyinc apps/nsmbridged

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/nsmbridged
	@ ln -s $(BELLE2_DAQ_SLC)/apps/nsmbridged/include/* $(INCDIR)/apps/nsmbridged/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/nsmbridged/include/* $(INCDIR)/apps/nsmbridged

apps/nsmbridged: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
