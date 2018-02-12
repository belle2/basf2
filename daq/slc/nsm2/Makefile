#
# 20131229  newly introduced
#

FILES = \
 Makefile \
 b2lib/Makefile \
 b2lib/belle2nsm.c \
 b2lib/belle2nsm.h \
 corelib/Makefile \
 corelib/nsm2.h \
 corelib/nsmhash.c \
 corelib/nsmlib2.c \
 corelib/nsmlib2.h \
 corelib/nsmparse.c \
 corelib/nsmparse.h \
 corelib/nsmsys2.h \
 nsmget/Makefile \
 nsmget/libnsmget.cc \
 nsmget/nsmget.h \
 daemon/Makefile \
 daemon/nsmd2.cc \
 daemon/nsmhash.c \
 daemon/nsminfo2.cc \
 daemon/nsmsys2.h \
 example/Makefile \
 example/Makefile.no-readline \
 example/anonymous.cc \
 example/bridge.cc \
 example/bridge_data.h \
 example/client.cc \
 example/client_data.h \
 example/client_wait.cc \
 example/master.cc \
 example/readdat.cc \
 example/nsmget.cc \
 doc/nsm2.pdf

VER = $(shell sed -n '/NSM_PACKAGE_VERSION/s/.*ION *\([0-9]*\) .*/\1/p' daemon/nsm2.h)

all:
	(cd daemon  ; make clean all)
	(cd corelib ; make clean all)
	(cd b2lib   ; make clean all)
	(cd nsmget  ; make clean all)
	(cd example ; make clean all ||\
	              make -f Makefile.no-readline clean all )

install: all
	(cd daemon  ; make install)
	(cd corelib ; make install)
	(cd b2lib   ; make install)
	(cd nsmget  ; make install)
	(cd example ; make install ||\
	              make -f Makefile.no-readline clean all )


clean:
	(cd daemon  ; make clean)
	(cd corelib ; make clean)
	(cd b2lib   ; make clean)
	(cd nsmget  ; make clean)
	(cd example ; make clean)

daemon/nsm2.h: $(FILES)
	@echo
	@echo =================================================
	@echo daemon/nsm2.h package version needs to be updated
	@echo
	@grep 'VERSION' daemon/nsm2.h
	@grep 'define.*VERSION' daemon/nsmd2.cc
	@grep 'nsmlib2_version' corelib/nsmlib2.c
	@grep 'belle2nsm_version' b2lib/belle2nsm.c
	@echo =================================================
	@echo
	@false

package: daemon/nsm2.h
	@grep 'VERSION' daemon/nsm2.h
	@grep 'define.*VERSION' daemon/nsmd2.cc
	@grep 'nsmlib2_version' corelib/nsmlib2.c
	@grep 'belle2nsm_version' b2lib/belle2nsm.c
	@echo
	@-rm -rf nsm2-$(VER)
	@mkdir nsm2-$(VER)
	@tar cf - daemon/nsm2.h $(FILES) | (cd nsm2-$(VER) ; tar xf -)
	@tar cf - nsm2-$(VER) | gzip -9 > nsm2-$(VER).tgz
	@-rm -rf nsm2-$(VER)
	@echo nsm2-$(VER).tgz is created

