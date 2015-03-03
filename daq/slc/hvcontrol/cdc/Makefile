all: copyinc hvcontrol/cdc

INCDIR=../../include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/cdc
	@  install -m 644 -p ../../hvcontrol/cdc/include/* $(INCDIR)/hvcontrol/cdc

hvcontrol/cdc: %:
	@ make -C ../.. -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi