all: copyinc apps/socketmonitord

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/socketmonitord
	@ ln -s $(BELLE2_DAQ_SLC)/apps/socketmonitord/include/* $(INCDIR)/apps/socketmonitord/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/socketmonitord/include/* $(INCDIR)/apps/socketmonitord

apps/socketmonitord: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
