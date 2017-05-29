all: copyinc apps/ttdad

INCDIR=../../include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/ttdad
	@ ln -s $(BELLE2_DAQ_SLC)/apps/ttdad/include/* $(INCDIR)/apps/ttdad/
#	@ install -m 644 -p ../../apps/ttdad/include/* $(INCDIR)/apps/ttdad

apps/ttdad: %:
	@ make -C ../.. -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
