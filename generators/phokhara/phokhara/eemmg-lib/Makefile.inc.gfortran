
# Makefile.inc for gcc + gfortran compilers

CXX = g++
F77 = gfortran

# create dynamic libraries
CXXFLAGS = -O2 -fPIC -DHAVE_CONFIG_H
FFLAGS = -O2 -fPIC -ff2c
LIB_LDFLAGS = -shared -Wl,-no-undefined -Wl,-export-dynamic
BIN_LDFLAGS = -Wl,-rpath=.
LIBEXT=.so

# create static libraries
#CXXFLAGS = -O2 -DHAVE_CONFIG_H
#FFLAGS = -O2 -ff2c
#LIB_LDFLAGS = -Wl,-no-undefined -Wl,-export-dynamic
#BIN_LDFLAGS = -lstdc++ -Wl,-Bstatic
#LIBEXT=.a

datarootdir = ${HOME}
LN_S = ln -s
SED = sed
SYMBOL_PREFIX =
HAVE_LD_VERSION_SCRIPT = yes
