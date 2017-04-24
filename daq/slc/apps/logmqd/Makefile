all: copyinc apps/logmqd

INCDIR=$(BELLE2_DAQ_SLC)/include/daq/slc/
CMS_INC=$(BELLE2_DAQ_SLC)/externals/activemq/include/activemq-cpp-3.9.3
APR_INC=$(BELLE2_DAQ_SLC)/externals/apr/include/apr-1

EXTERNAL_CFLAGS=-I$(APR_INC)
EXTERNAL_CFLAGS2=-I$(CMS_INC)

copyinc:
	@ mkdir -p $(INCDIR)/apps/logmqd
	@ ln -s $(BELLE2_DAQ_SLC)/apps/logmqd/include/* $(INCDIR)/apps/logmqd/
#	@ install -m 644 -p $(BELLE2_DAQ_SLC)/apps/logmqd/include/* $(INCDIR)/apps/logmqd

apps/logmqd: %:
	@ make -C $(BELLE2_DAQ_SLC) -f Makefile.src TARGET=$@ EXTERNAL_CFLAGS=$(EXTERNAL_CFLAGS) EXTERNAL_CFLAGS2=$(EXTERNAL_CFLAGS2)
	@ if [ -f ./tools/Makefile ]; then make -C ./tools; fi
