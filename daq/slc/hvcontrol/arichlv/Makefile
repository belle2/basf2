all: copyinc hvcontrol/arichlv

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/arichlv
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/arichlv/include/* $(INCDIR)/hvcontrol/arichlv

hvcontrol/arichlv: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi