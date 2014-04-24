#
# 20131229  newly introduced
#

FILES = \
 Makefile \
 b2lib/Makefile \
 b2lib/belle2nsm.c \
 b2lib/belle2nsm.h \
 b2lib/client_data.h \
 b2lib/main.c \
 corelib/Makefile \
 corelib/nsm2.h \
 corelib/nsmhash.c \
 corelib/nsmlib2.c \
 corelib/nsmlib2.h \
 corelib/nsmparse.c \
 corelib/nsmparse.h \
 corelib/nsmreq2.h \
 corelib/nsmsys2.h \
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
 doc/nsm2.pdf

all:
	(cd daemon  ; make clean all)
	(cd corelib ; make clean all)
	(cd b2lib   ; make clean all)
	(cd example ; make clean all ||\
	              make -f Makefile.no-readline clean all )

clean:
	(cd daemon  ; make clean)
	(cd corelib ; make clean)
	(cd b2lib   ; make clean)
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
	@-mkdir tgz
	@grep 'VERSION' daemon/nsm2.h
	@grep 'define.*VERSION' daemon/nsmd2.cc
	@grep 'nsmlib2_version' corelib/nsmlib2.c
	@grep 'belle2nsm_version' b2lib/belle2nsm.c
	@echo
	tar cf - daemon/nsm2.h $(FILES)  | gzip -9 \
	> tgz/nsm2-$(shell sed -n '/NSM_PACKAGE_VERSION/s/.*ION *\([0-9]*\) .*/\1/p' daemon/nsm2.h).tgz
