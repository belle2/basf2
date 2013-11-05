.PHONY : all clean base system nsm dqm xml database

SRCDIR=../src
INCDIR=../include
LIBDIR=../lib
TMPDIR=../tmp

all: base nsm system

full: all xml database

base:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $(SRCDIR)/$@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

system:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $(SRCDIR)/$@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

dqm:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $(SRCDIR)/$@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

nsm:
	@ mkdir -p $(INCDIR)/$@
	@ if [ ! -e $(LIBDIR)/libnsm2_corelib.so ]; then make -C ../nsm2/; fi
	@ install -m 644 -p $(SRCDIR)/$@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@

xml:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $(SRCDIR)/$@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ EXTERNAL_CFLAGS=$(shell xml2-config --cflags)

database:
	@ mkdir -p $(INCDIR)/$@
	@ install -m 644 -p $(SRCDIR)/$@/include/* $(INCDIR)/$@
	@ make -f Makefile.src TARGET=$@ EXTERNAL_CFLAGS=\
	"$(shell xml2-config --cflags) $(shell mysql_config --cflags)"

clean:
	@ rm -rf $(INCDIR)/* $(LIBDIR)/*.so $(TMPDIR)
	@ make -C ../nsm2 clean
	@ rm -rf *~ \#*~ .*~ */*~ */*/*~

# LocalWords:  gui
