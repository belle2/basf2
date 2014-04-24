.PHONY : all clean base system database nsm dqm runcontrol

SRCDIR=./src
INCDIR=./include/daq/slc/
LIBDIR=./lib
TMPDIR=./tmp

all: base system database nsm runcontrol
	@ make -C extra/tools

full: all

base:
	@ mkdir -p $(INCDIR)/$@
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
	@ mkdir -p bin
	@ make -C ./extra/nsm2/
	@ make -C ./extra/nsm2/daemon 
	@ make -C ./extra/nsm2/daemon install EXTBINDIR=$(PWD)/bin EXTINCDIR=$(PWD)/include
	@ install -m 644 -p ./extra/nsm2/corelib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/b2lib/*.h include/nsm2
	@ install -m 644 -p ./extra/nsm2/corelib/*.so $(LIBDIR)
	@ install -m 644 -p ./extra/nsm2/b2lib/*.so $(LIBDIR)
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

runcontrol:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ 

database:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ EXTERNAL_CFLAGS=\
	"$(shell xml2-config --cflags) $(shell mysql_config --cflags)"

clean:
	@ rm -rf include $(LIBDIR) $(TMPDIR) bin
	@ make -C ./extra clean
	@ make -C ./extra/nsm2/example clean
	@ rm -rf *~ \#*~ .*~ */*~ */*/*~

# LocalWords:  gui
