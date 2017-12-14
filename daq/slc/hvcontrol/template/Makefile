all: copyinc hvcontrol/template

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/template
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/template/include/* $(INCDIR)/hvcontrol/template

hvcontrol/template: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi