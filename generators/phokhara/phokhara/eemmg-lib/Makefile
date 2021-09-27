
include Makefile.inc

# -------------------------------------------------------------------------
# eemmg5 library
libeemmg5_SOURCES = eemmg5/sqampeemmg-main.f eemmg5/sqampeemmg-wrap.f eemmg5/sqampeemmg-1.f \
                    eemmg5/sqampeemmg-2.f eemmg5/sqampeemmg-3.f eemmg5/sqampeemmg-4.f eemmg5/sqampeemmg-5.f
libeemmg5_OBJECTS := $(libeemmg5_SOURCES:%.f=%.o)
libeemmg5_LDFLAGS := $(LIB_LDFLAGS) -L. -lpjfry -lqcdloop1
ifeq ($(HAVE_LD_VERSION_SCRIPT),yes)
  # Versioned symbols and restricted exports
  libeemmg5_LDFLAGS += -Wl,--version-script=libeemmg5.vers
  libeemmg5_DEPENDENCIES = libeemmg5.vers
endif

# -------------------------------------------------------------------------
# eemmgisr library
libeemmgisr_SOURCES = eemmgisr/sqampeemmg-main.f eemmgisr/sqampeemmg-wrap.f eemmgisr/sqampeemmg-1.f
libeemmgisr_OBJECTS := $(libeemmgisr_SOURCES:%.f=%.o)
libeemmgisr_LDFLAGS := $(LIB_LDFLAGS) -L. -lpjfry -lqcdloop1
ifeq ($(HAVE_LD_VERSION_SCRIPT),yes)
  # Versioned symbols and restricted exports
  libeemmgisr_LDFLAGS += -Wl,--version-script=libeemmgisr.vers
  libeemmgisr_DEPENDENCIES = libeemmgisr.vers
endif

# -------------------------------------------------------------------------
# eemmgisrfsr library
libeemmgisrfsr_SOURCES = eemmgisrfsr/sqampeemmg-main.f eemmgisrfsr/sqampeemmg-wrap.f eemmgisrfsr/sqampeemmg-1.f
libeemmgisrfsr_OBJECTS := $(libeemmgisrfsr_SOURCES:%.f=%.o)
libeemmgisrfsr_LDFLAGS := $(LIB_LDFLAGS) -L. -lpjfry -lqcdloop1
ifeq ($(HAVE_LD_VERSION_SCRIPT),yes)
  # Versioned symbols and restricted exports
  libeemmgisrfsr_LDFLAGS += -Wl,--version-script=libeemmgisrfsr.vers
  libeemmgisrfsr_DEPENDENCIES = libeemmgisrfsr.vers
endif

# -------------------------------------------------------------------------
# eemmgmix library
libeemmgmix_SOURCES = eemmgmix/sqampeemmg-main.f eemmgmix/sqampeemmg-wrap.f eemmgmix/sqampeemmg-1.f
libeemmgmix_OBJECTS := $(libeemmgmix_SOURCES:%.f=%.o)
libeemmgmix_LDFLAGS := $(LIB_LDFLAGS) -L. -lpjfry -lqcdloop1
ifeq ($(HAVE_LD_VERSION_SCRIPT),yes)
  # Versioned symbols and restricted exports
  libeemmgmix_LDFLAGS += -Wl,--version-script=libeemmgmix.vers
  libeemmgmix_DEPENDENCIES = libeemmgmix.vers
endif

# -------------------------------------------------------------------------
# PJFry demo program
demo_SOURCES = demo.f
demo_OBJECTS := $(demo_SOURCES:%.f=%.o)
demo_LDFLAGS := $(BIN_LDFLAGS) -L. -lpjfry -lqcdloop1 -Wl,-Bdynamic

# -------------------------------------------------------------------------
# PJFry library
libpjfry_SOURCES = ./src/integral.cpp ./src/minor.cpp ./src/minorex.cpp\
                   ./src/pjfry.cpp ./src/cache.cpp ./src/minoreval.cpp
libpjfry_OBJECTS := $(libpjfry_SOURCES:%.cpp=%.o)
libpjfry_LDFLAGS := $(LIB_LDFLAGS) -L. -lqcdloop1
ifeq ($(HAVE_LD_VERSION_SCRIPT),yes)
  # Versioned symbols and restricted exports
  libpjfry_LDFLAGS += -Wl,--version-script=libpjfry.vers
  libpjfry_DEPENDENCIES = libpjfry.vers
endif

# -------------------------------------------------------------------------
# QCDLoop test program
test_SOURCES = test.f
test_OBJECTS := $(test_SOURCES:%.f=%.o)
test_LDFLAGS := $(BIN_LDFLAGS) -L. -lqcdloop1 -Wl,-Bdynamic

# -------------------------------------------------------------------------
# QCDLoop library
libqcdloop1_SOURCES = ff/ffxc0.f ff/ffxd0.f \
    ff/fftran.f ff/ffxd0h.f ff/ffdel2.f ff/ffdel3.f ff/ffdel4.f \
    ff/ff2dl2.f ff/ffabcd.f ff/ffxc0i.f ff/ffxc0p.f ff/ffxd0p.f ff/ffxdbd.f \
    ff/ffxli2.f ff/ffcli2.f ff/ffcxs3.f ff/ffcxs4.f ff/ffxxyz.f ff/ffcxyz.f \
    ff/ffcc0p.f ff/ffcxr.f ff/ffcrr.f ff/ffdxc0.f ff/ffdcc0.f ff/ffxd0i.f \
    ff/ffcel2.f ff/ffcel3.f ff/ffdcxs.f \
    ql/qlinit.f ql/qlI1.f ql/qlI2.f ql/qlI3.f ql/qlI4.f \
    ql/qlI2fin.f ql/qlI3fin.f ql/qlI4fin.f \
    ql/qlI3sub.f ql/qlI4array.f ql/qlI4sub0m.f ql/qlI4sub1m.f \
    ql/qlI4sub2m.f ql/qlI4sub2ma.f ql/qlI4sub2mo.f ql/qlI4sub3m.f \
    ql/auxCD.f ql/qlzero.f ql/qltrisort.f ql/qlsnglsort.f ql/qlfndd.f \
    ql/qlkfn.f ql/qllnrat.f ql/qlYcalc.f ql/ddilog.f \
    ql/qltri1.f ql/qltri2.f ql/qltri3.f ql/qltri4.f ql/qltri5.f ql/qltri6.f \
    ql/qlbox1.f ql/qlbox2.f ql/qlbox3.f ql/qlbox4.f ql/qlbox5.f ql/qlbox6.f \
    ql/qlbox7.f ql/qlbox8.f ql/qlbox9.f ql/qlbox10.f \
    ql/qlbox11.f ql/qlbox12.f ql/qlbox13.f ql/qlbox14.f ql/qlbox15.f ql/qlbox16.f \
    ql/qlfunctions.f ql/qlratgam.f ql/qlratreal.f ql/qlspencer.f ql/qlI4DNS41.f ql/qlxpicheck.f \
    ql/qlcLi2omx2.f ql/qlcLi2omx3.f ql/qlLi2omprod.f ql/qlLi2omrat.f ql/qlLi2omx.f ql/qlLi2omx2.f \
    ffinit_auto.f
libqcdloop1_OBJECTS := $(libqcdloop1_SOURCES:%.f=%.o)
EXTRA_libqcdloop1_SOURCES = ff/ffinit_mine.f ff/ff.h ff/aa.h ql/qlconstants.f ql/qlonshellcutoff.f
libqcdloop1_LDFLAGS := $(LIB_LDFLAGS)
ifeq ($(HAVE_LD_VERSION_SCRIPT),yes)
  # Versioned symbols and restricted exports
  libqcdloop1_LDFLAGS += -Wl,--version-script=libqcdloop1.vers
  libqcdloop1_DEPENDENCIES = libqcdloop1.vers
endif
nobase_dist_data_DATA = ff/fferr.dat ff/ffwarn.dat
CLEANFILES = ffinit_auto.f ff_auto.h aa_auto.h

# -------------------------------------------------------------------------

TARGETS = libeemmg5$(LIBEXT) libeemmgisr$(LIBEXT) libeemmgisrfsr$(LIBEXT) libeemmgmix$(LIBEXT) test demo

all: $(TARGETS)

test: $(test_OBJECTS) libqcdloop1$(LIBEXT)
	$(F77) -o $@ $(test_OBJECTS) $(test_LDFLAGS)

demo: $(demo_OBJECTS) libpjfry$(LIBEXT)
	$(F77) -o $@ $(demo_OBJECTS) $(demo_LDFLAGS)

ifeq ($(LIBEXT),.a)
libeemmg5$(LIBEXT): $(libeemmg5_OBJECTS) $(libeemmg5_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	ar cru $@ $(libeemmg5_OBJECTS)

libeemmgisr$(LIBEXT): $(libeemmgisr_OBJECTS) $(libeemmgisr_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	ar cru $@ $(libeemmgisr_OBJECTS)

libeemmgisrfsr$(LIBEXT): $(libeemmgisrfsr_OBJECTS) $(libeemmgisrfsr_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	ar cru $@ $(libeemmgisrfsr_OBJECTS)

libeemmgmix$(LIBEXT): $(libeemmgmix_OBJECTS) $(libeemmgmix_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	ar cru $@ $(libeemmgmix_OBJECTS)

libpjfry$(LIBEXT): $(libpjfry_OBJECTS) $(libpjfry_DEPENDENCIES) libqcdloop1$(LIBEXT)
	ar cru $@ $(libpjfry_OBJECTS)

libqcdloop1$(LIBEXT): $(libqcdloop1_OBJECTS) $(libqcdloop1_DEPENDENCIES)
	ar cru $@ $(libqcdloop1_OBJECTS)
else
libeemmg5$(LIBEXT): $(libeemmg5_OBJECTS) $(libeemmg5_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	$(F77) -o $@ $(libeemmg5_OBJECTS) $(libeemmg5_LDFLAGS)

libeemmgisr$(LIBEXT): $(libeemmgisr_OBJECTS) $(libeemmgisr_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	$(F77) -o $@ $(libeemmgisr_OBJECTS) $(libeemmgisr_LDFLAGS)

libeemmgisrfsr$(LIBEXT): $(libeemmgisrfsr_OBJECTS) $(libeemmgisrfsr_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	$(F77) -o $@ $(libeemmgisrfsr_OBJECTS) $(libeemmgisrfsr_LDFLAGS)

libeemmgmix$(LIBEXT): $(libeemmgmix_OBJECTS) $(libeemmgmix_DEPENDENCIES) libpjfry$(LIBEXT) libqcdloop1$(LIBEXT)
	$(F77) -o $@ $(libeemmgmix_OBJECTS) $(libeemmgmix_LDFLAGS)

libpjfry$(LIBEXT): $(libpjfry_OBJECTS) $(libpjfry_DEPENDENCIES) libqcdloop1$(LIBEXT)
	$(CXX) -o $@ $(libpjfry_OBJECTS) $(libpjfry_LDFLAGS)

libqcdloop1$(LIBEXT): $(libqcdloop1_OBJECTS) $(libqcdloop1_DEPENDENCIES)
	$(F77) -o $@ $(libqcdloop1_OBJECTS) $(libqcdloop1_LDFLAGS)
endif

ffinit_auto.f: ff/ffinit_mine.f ff_auto.h aa_auto.h
	rm -f $@ $@.new
	$(SED) 's|/usr/local/share/ff/|$(datarootdir)/ff/|' $< | \
	$(SED) "s|^\([\t ]\{1,5\}\)\(.\{66\}\)\(.\{1,\}\)$$|\1\2\n     +\3|" | \
	$(SED) "s|include 'ff.h'|include 'ff_auto.h'|"  | \
	$(SED) "s|include 'aa.h'|include 'aa_auto.h'|"  >> $@.new
	mv $@.new $@

ff_auto.h: ff/ff.h
	$(LN_S) $< $@

aa_auto.h: ff/aa.h
	$(LN_S) $< $@

%.o: %.f
	$(F77) $(FFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) *.a *.so $(CLEANFILES) $(libqcdloop1_OBJECTS) $(libpjfry_OBJECTS) $(test_OBJECTS) $(demo_OBJECTS)

