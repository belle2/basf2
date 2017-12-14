all: copyinc copper/dummy

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/dummy
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/dummy/include/* $(INCDIR)/copper/dummy

copper/dummy: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
