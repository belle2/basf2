all: copyinc apps/runcontrold

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/runcontrold
	@ ln -s $(BELLE2_DAQ_SLC)/apps/runcontrold/include/* $(INCDIR)/apps/runcontrold/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/runcontrold/include/* $(INCDIR)/apps/runcontrold

apps/runcontrold: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi