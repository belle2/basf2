all: copyinc apps/logreaderd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/logreaderd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/logreaderd/include/* $(INCDIR)/apps/logreaderd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/logreaderd/include/* $(INCDIR)/apps/logreaderd

apps/logreaderd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
