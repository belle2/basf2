.PHONY : all clean base system database nsm dqm runcontrol hvcontrol hvcontrol/arich hvcontrol/cdc apps/nsm2socket

SRCDIR=./src
INCDIR=./include/daq/slc/
LIBDIR=./lib
BINDIR=./bin
TMPDIR=./tmp

all: base system database nsm nsm/tools runcontrol hvcontrol hvcontrol/arich hvcontrol/cdc apps/nsm2socket

full: all

base:
	@ mkdir -p $(INCDIR)/$@
	@ mkdir -p bin
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

system:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

dqm:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

nsm:
	@ mkdir -p $(INCDIR)/$@
	@ mkdir -p include/nsm2
	@ make -C ./extra/nsm2
	@ install -m 644 -p ./extra/nsm2/corelib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/b2lib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/corelib/*.so $(LIBDIR)
	@ install -m 644 -p ./extra/nsm2/b2lib/*.so $(LIBDIR)
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ install -m 755 -p ./extra/nsm2/daemon/nsmd2 $(BINDIR)/
	@ install -m 755 -p ./extra/nsm2/daemon/nsminfo2 $(BINDIR)/
	@ make -f Makefile.src TARGET=$@
	@ make -C nsm/tools

runcontrol:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ 

hvcontrol:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ 
	@ make -C hvcontrol/tools

hvcontrol/arich:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ 
	@ make -C $@/tools

hvcontrol/cdc:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ 
	@ make -C $@/tools

database:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ 
	@ make -C database/tools

apps/nsm2socket:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ 
	@ make -C apps/nsm2socket/tools

clean:
	@ rm -rf include $(LIBDIR) $(TMPDIR) bin
	@ make -C ./extra clean
	@ make -C ./extra/nsm2/example clean
	@ rm -rf *~ \#*~ .*~ */*~ */*/*~

# LocalWords:  gui
