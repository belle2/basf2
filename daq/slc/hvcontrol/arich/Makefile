all: copyinc hvcontrol/arich

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/arich
	@ install -m 644 -p $(BELLE2_DAQ_SLC)/hvcontrol/arich/include/* $(INCDIR)/hvcontrol/arich

hvcontrol/arich: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
