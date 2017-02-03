all: copyinc apps/ttdad

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/ttdad
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/ttdad/include/* $(INCDIR)/apps/ttdad

apps/ttdad: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
