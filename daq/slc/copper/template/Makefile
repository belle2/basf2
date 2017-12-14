all: copyinc copper/template

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/copper/template
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/copper/template/include/* $(INCDIR)/copper/template

copper/template: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
