all: copyinc apps/ecldspd

INCDIR=../../include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/ecldspd
	@  install -m 644 -p ../../apps/ecldspd/include/* $(INCDIR)/apps/ecldspd

apps/ecldspd: %:
	@ make -C ../.. -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
