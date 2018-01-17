all: copyinc apps/arichtempd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/arichtempd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/arichtempd/include/* $(INCDIR)/apps/arichtempd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/arichtempd/include/* $(INCDIR)/apps/arichtempd

apps/arichtempd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
