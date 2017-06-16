all: copyinc apps/nsmtemplated

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/nsmtemplated
	@ ln -s $(BELLE2_DAQ_SLC)/apps/nsmtemplated/include/* $(INCDIR)/apps/nsmtemplated/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/nsmtemplated/include/* $(INCDIR)/apps/nsmtemplated

apps/nsmtemplated: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
