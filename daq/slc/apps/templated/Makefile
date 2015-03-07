all: copyinc apps/templated

INCDIR=../../include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/templated
	@  install -m 644 -p ../../apps/templated/include/* $(INCDIR)/apps/templated

apps/templated: %:
	@ make -C ../.. -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
