all: copyinc apps/cprcontrold

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/cprcontrold
	@ ln -s $(BELLE2_DAQ_SLC)/apps/cprcontrold/include/* $(INCDIR)/apps/cprcontrold
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/cprcontrold/include/* $(INCDIR)/apps/cprcontrold

apps/cprcontrold: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
