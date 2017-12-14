all: copyinc apps/mwreaderd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/mwreaderd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/mwreaderd/include/* $(INCDIR)/apps/mwreaderd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/mwreaderd/include/* $(INCDIR)/apps/mwreaderd

apps/mwreaderd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
