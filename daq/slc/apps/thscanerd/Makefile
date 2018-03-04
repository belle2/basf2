all: copyinc apps/thscanerd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/thscanerd
	@ ln -sf $(BELLE2_DAQ_SLC)/apps/thscanerd/include/* $(INCDIR)/apps/thscanerd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/thscanerd/include/* $(INCDIR)/apps/thscanerd

apps/thscanerd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
