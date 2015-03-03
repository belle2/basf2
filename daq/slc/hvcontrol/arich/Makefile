all: copyinc hvcontrol/arich

INCDIR=../../include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/arich
	@  install -m 644 -p ../../hvcontrol/arich/include/* $(INCDIR)/hvcontrol/arich

hvcontrol/arich: %:
	@ make -C ../.. -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi