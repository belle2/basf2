all: copyinc apps/hltscriptd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/hltscriptd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/hltscriptd/include/* $(INCDIR)/apps/hltscriptd
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/hltscriptd/include/* $(INCDIR)/apps/hltscriptd

apps/hltscriptd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
