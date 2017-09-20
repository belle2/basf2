all: copyinc apps/eb1rxd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/eb1rxd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/eb1rxd/include/* $(INCDIR)/apps/eb1rxd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/eb1rxd/include/* $(INCDIR)/apps/eb1rxd

apps/eb1rxd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
