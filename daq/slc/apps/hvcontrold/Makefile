all: copyinc apps/hvcontrold

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/hvcontrold
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/hvcontrold/include/* $(INCDIR)/apps/hvcontrold

apps/hvcontrold: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi