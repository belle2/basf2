all: copyinc copper/svdftb

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/svdftb
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/svdftb/include/* $(INCDIR)/copper/svdftb

copper/svdftb: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
