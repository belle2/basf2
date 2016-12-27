all: copyinc apps/errdiagd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/errdiagd
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/errdiagd/include/* $(INCDIR)/apps/errdiagd

apps/errdiagd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
