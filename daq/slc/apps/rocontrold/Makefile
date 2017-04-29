all: copyinc apps/rocontrold

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/rocontrold
	@ ln -s $(BELLE2_DAQ_SLC)/apps/rocontrold/include/* $(INCDIR)/apps/rocontrold/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/rocontrold/include/* $(INCDIR)/apps/rocontrold

apps/rocontrold: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
