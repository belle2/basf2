all: copyinc apps/databrigded

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/databrigded
	@ ln -s $(BELLE2_DAQ_SLC)/apps/databrigded/include/* $(INCDIR)/apps/databrigded/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/databrigded/include/* $(INCDIR)/apps/databrigded

apps/databrigded: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
