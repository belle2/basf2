all: copyinc copper/ecl

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/ecl
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/ecl/include/* $(INCDIR)/copper/ecl

copper/ecl: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
