all: copyinc apps/sockmemd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/sockmemd
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/sockmemd/include/* $(INCDIR)/apps/sockmemd

apps/sockmemd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
