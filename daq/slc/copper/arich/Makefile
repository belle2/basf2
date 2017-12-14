all: copyinc copper/arich

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/arich
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/arich/include/* $(INCDIR)/copper/arich

copper/arich: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
