all: copyinc apps/eb2txd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/eb2txd
	@ ln -sf $(BELLE2_DAQ_SLC)/apps/eb2txd/include/* $(INCDIR)/apps/eb2txd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/eb1rxd/include/* $(INCDIR)/apps/eb1rxd

apps/eb2txd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
