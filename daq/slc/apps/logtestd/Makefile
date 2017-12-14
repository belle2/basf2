all: copyinc apps/logtestd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/logtestd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/logtestd/include/* $(INCDIR)/apps/logtestd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/logtestd/include/* $(INCDIR)/apps/logtestd

apps/logtestd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
