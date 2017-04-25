all: copyinc apps/hvmasterd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/hvmasterd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/hvmasterd/include/* $(INCDIR)/apps/hvmasterd
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/hvmasterd/include/* $(INCDIR)/apps/hvmasterd

apps/hvmasterd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
