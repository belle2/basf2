all: copyinc hvcontrol/cdc

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/cdc
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/cdc/include/* $(INCDIR)/hvcontrol/cdc

hvcontrol/cdc: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi