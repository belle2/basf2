all: copyinc apps/py2nsmd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/py2nsmd
	@  install -m 644 -p $(BELLE2_DAQ_SLC)/apps/py2nsmd/include/* $(INCDIR)/apps/py2nsmd

apps/py2nsmd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
