all: copyinc copper/klm

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/klm
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/klm/include/* $(INCDIR)/copper/klm

copper/klm: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
