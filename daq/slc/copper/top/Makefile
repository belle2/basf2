all: copyinc copper/top

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/top
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/top/include/* $(INCDIR)/copper/top

copper/top: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
