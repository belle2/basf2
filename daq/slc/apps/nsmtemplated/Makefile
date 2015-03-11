all: copyinc apps/nsmtemplated

INCDIR=../../include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/nsmtemplated
	@  install -m 644 -p ../../apps/nsmtemplated/include/* $(INCDIR)/apps/nsmtemplated

apps/nsmtemplated: %:
	@ make -C ../.. -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
