all: copyinc copper/trg

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/trg
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/trg/include/* $(INCDIR)/copper/trg

copper/trg: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
