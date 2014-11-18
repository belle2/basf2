.PHONY : all clean base system database nsm readout dqm runcontrol hvcontrol hvcontrol/arich hvcontrol/cdc apps/nsm2socket apps/runcontrold apps apps/rocontrold apps/cprcontrold apps/eclcollectord

SRCDIR=./src
INCDIR=./include/daq/slc/
LIBDIR=./lib
BINDIR=./bin
TMPDIR=./tmp

APPS = apps/nsm2socket apps/runcontrold apps/rocontrold apps/cprcontrold apps/eclcollectord

SUBPACKAGES = base system database nsm readout runcontrol hvcontrol hvcontrol/arich hvcontrol/cdc $(APPS)

all: copyinc $(SUBPACKAGES)
	@ make -C tools/

full: all

copyinc:
	@ mkdir -p bin
	@ for dir in $(SUBPACKAGES); \
	  do mkdir -p $(INCDIR)/$$dir; install -m 644 -p $$dir/include/* $(INCDIR)/$$dir; done
	@ mkdir -p lib
	@ mkdir -p include/nsm2
	@ make -C ./extra/nsm2
	@ install -m 644 -p ./extra/nsm2/corelib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/b2lib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/corelib/*.so $(LIBDIR)
	@ install -m 644 -p ./extra/nsm2/b2lib/*.so $(LIBDIR)
	@ install -m 755 -p ./extra/nsm2/daemon/nsmd2 $(BINDIR)
	@ install -m 755 -p ./extra/nsm2/daemon/nsminfo2 $(BINDIR)
	@ mkdir -p include/mgt
	@ make -C extra/mgt BELLE2_EXTERNALS_DIR=$(PWD)
	@ mkdir -p include/ttrx
	@ make -C extra/ttrx BELLE2_EXTERNALS_DIR=$(PWD)
	@ mkdir -p include/ecl
	@ make -C extra/ecl BELLE2_EXTERNALS_DIR=$(PWD)

$(SUBPACKAGES): %:
	@ make -f Makefile.src TARGET=$@
	@ if [ -f $@/tools/Makefile ]; then make -C $@/tools; fi

clean:
	@ rm -rf include $(LIBDIR) $(TMPDIR) bin
	@ make -C ./extra clean
	@ make -C ./extra/nsm2/example clean
	@ rm -rf *~ \#*~ .*~ */*~ */*/*~

# LocalWords:  gui
