all: copyinc apps/ttctrld

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/

copyinc:
	@ mkdir -p $(INCDIR)/apps/ttctrld
	@ ln -s $(BELLE2_DAQ_SLC)/apps/ttctrld/include/* $(INCDIR)/apps/ttctrld/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/ttctrld/include/* $(INCDIR)/apps/ttctrld

apps/ttctrld: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@ 
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
