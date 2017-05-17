all: copyinc apps/ecldspd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/ecldspd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/ecldspd/include/* $(INCDIR)/apps/ecldspd
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/ecldspd/include/* $(INCDIR)/apps/ecldspd

apps/ecldspd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
