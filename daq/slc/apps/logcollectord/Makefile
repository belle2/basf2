all: copyinc apps/logcollectord

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/logcollectord
	@ ln -sf $(BELLE2_DAQ_SLC)/apps/logcollectord/include/* $(INCDIR)/apps/logcollectord/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/logcollectord/include/* $(INCDIR)/apps/logcollectord

apps/logcollectord: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
