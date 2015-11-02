.PHONY : all clean tools base system database nsm readout copper dqm runcontrol hvcontrol apps/nsm2socket

SRCDIR=./src
INCDIR=./include/daq/slc/
LIBDIR=./lib
BINDIR=./bin
TMPDIR=./tmp

SUBPACKAGES = base system database nsm readout copper runcontrol hvcontrol apps/nsm2socket

all: copyinc $(SUBPACKAGES) tool
	@ make -C tools/
	@ install -m 755 -p ./tools/*.sh ${BINDIR}/

full: all

runcontrold:
	@ make -C apps/runcontrold

nsm2socket:
	@ make -C apps/nsm2socket

rctemplated:
	@ make -C apps/rctemplated

copyinc:
	@ mkdir -p bin
	@ mkdir -p lib
	@ for dir in $(SUBPACKAGES); \
	  do mkdir -p $(INCDIR)/$$dir; install -m 644 -p $$dir/include/* $(INCDIR)/$$dir; done
	@ mkdir -p include/nsm2
	@ make nsmlib
	@ mkdir -p include/mgt
	@ make -C extra/mgt 
	@ mkdir -p include/ttrx
	@ make -C extra/ttrx 
	@ mkdir -p include/ecldaq
	@ make -C extra/ecldaq 

$(SUBPACKAGES): %:
	@ make -f Makefile.src TARGET=$@ NOT_USE_PSQL=$(NOT_USE_PSQL)
	@ if [ $@ = "copper" ]; then make -C copper; fi

tool: 
	@ for dir in $(SUBPACKAGES); \
	 do if [ -f $$dir/tools/Makefile ]; then make -C $$dir/tools NOT_USE_PSQL=$(NOT_USE_PSQL); fi done

clean:
	@ rm -rf include $(LIBDIR) $(TMPDIR) bin
	@ make -C ./extra clean
	@ make -C ./extra/nsm2/example clean
	@ rm -rf *~ \#*~ .*~ */*~ */*/*~

nsmlib: lib/libnsm2_b2lib.so lib/libnsm2_corelib.so nsmlib_daemon 

nsmlib_b2lib: lib/libnsm2_b2lib.so

lib/libnsm2_b2lib.so: extra/nsm2/b2lib/*.h extra/nsm2/b2lib/*.c
	@ make -C extra/nsm2/b2lib
	@ install -m 644 -p ./extra/nsm2/b2lib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/b2lib/*.so $(LIBDIR)

nsmlib_corelib: lib/libnsm2_corelib.so

lib/libnsm2_corelib.so: extra/nsm2/corelib/*.c 
	@ make -C extra/nsm2/corelib
	@ install -m 644 -p ./extra/nsm2/corelib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/corelib/*.so $(LIBDIR)

nsmlib_daemon: $(BINDIR)/nsmd2 $(BINDIR)/nsminfo2

$(BINDIR)/nsmd2: extra/nsm2/daemon/nsmd2.cc 
	@ make -C extra/nsm2/daemon
	@ install -m 755 -p ./extra/nsm2/daemon/nsmd2 $(BINDIR)

$(BINDIR)/nsminfo2: extra/nsm2/daemon/nsminfo2.cc 
	@ install -m 755 -p ./extra/nsm2/daemon/nsminfo2 $(BINDIR)


