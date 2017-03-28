all: copyinc apps/rctemplated

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/rctemplated
	@ ln -s $(BELLE2_DAQ_SLC)/apps/rctemplated/include/* $(INCDIR)/apps/rctemplated/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/rctemplated/include/* $(INCDIR)/apps/rctemplated

apps/rctemplated: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
