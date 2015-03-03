all: copyinc hvcontrol/template

INCDIR=../../include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/hvcontrol/template
	@  install -m 644 -p ../../hvcontrol/template/include/* $(INCDIR)/hvcontrol/template

hvcontrol/template: %:
	@ make -C ../.. -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi