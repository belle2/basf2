all: copyinc copper/cdc

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/cdc
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/cdc/include/* $(INCDIR)/copper/cdc

copper/cdc: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
