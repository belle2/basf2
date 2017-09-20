all: copyinc apps/rc2eb1d

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/rc2eb1d
	@ ln -sf $(BELLE2_DAQ_SLC)/apps/rc2eb1d/include/* $(INCDIR)/apps/rc2eb1d/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/rc2eb1d/include/* $(INCDIR)/apps/rc2eb1d

apps/rc2eb1d: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
