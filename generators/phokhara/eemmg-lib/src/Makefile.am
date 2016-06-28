## Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libpjfry.la

libpjfry_la_SOURCES = minor.cpp minorex.cpp minoreval.cpp cache.cpp integral.cpp pjfry.cpp
EXTRA_libpjfry_la_SOURCES = common.h minor.h cache.h integral.h pjfry.h kinem.h pointer.h golem.h

nodist_EXTRA_libpjfry_la_SOURCES = remove.cpp

libpjfry_la_LDFLAGS = -no-undefined -export-dynamic \
	-version-info @PJFRY_VERSION_INFO@

if HAVE_FLIBS
  libpjfry_la_LIBADD = $(FLIBS)
endif

if EMU_GOLEM
  libpjfry_la_SOURCES += golem.cpp
endif

if HAVE_LD_VERSION_SCRIPT
  # Versioned symbols and restricted exports
  libpjfry_la_LDFLAGS += -Wl,--version-script=${builddir}/libpjfry.vers
  libpjfry_la_DEPENDENCIES = ${builddir}/libpjfry.vers
else
  # Skip because '-export-symbols' doesn't work with C++
endif

include_HEADERS = pjfry.h

# check_PROGRAMS = test
