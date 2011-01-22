
# define directories
export EXTDIR :=  $(shell pwd)
export EXTINCDIR := $(EXTDIR)/include
export EXTLIBDIR := $(EXTDIR)/lib/$(BELLE2_SUBDIR)
export EXTBINDIR := $(EXTDIR)/bin/$(BELLE2_SUBDIR)

export GENFIT := $(EXTDIR)/genfit


# all target
all: boost clhep geant4 root genfit evtgen

# clean up target
clean:
	@cd root; make clean
	@cd boost; ./bjam --clean

# directory creation
$(EXTINCDIR):
	@echo "create  $(EXTINCDIR)"
	@mkdir -p $(EXTINCDIR)

$(EXTLIBDIR):
	@echo "create  $(EXTLIBDIR)"
	@mkdir -p $(EXTLIBDIR)

$(EXTBINDIR):
	@echo "create  $(EXTBINDIR)"
	@mkdir -p $(EXTBINDIR)


# dependence for boost build
boost: boost/project-config.jam

# boost build command
boost/project-config.jam:
	@echo "building boost"
	@cd boost; ./bootstrap.sh --includedir=$(EXTINCDIR) --libdir=$(EXTLIBDIR); ./bjam install

# dependence for CLHEP build
clhep: CLHEP/config.log

# dependence for CLHEP download
CLHEP/configure:
	@echo "downloading CLHEP"
	@wget -O - http://proj-clhep.web.cern.ch/proj-clhep/DISTRIBUTION/tarFiles/clhep-2.0.4.5.tgz | tar xz
	@mv 2.0.4.5/CLHEP .
	@rmdir 2.0.4.5

# CLHEP build command
CLHEP/config.log: CLHEP/configure
	@echo "building CLHEP"
	@cd CLHEP; ./configure --prefix=$(EXTDIR) \
	--includedir=$(EXTINCDIR) --libdir=$(EXTLIBDIR) --bindir=$(EXTBINDIR); make; make install

# dependence for GEANT4 build
geant4: geant4/env.sh

# dependence for GEANT4 download
geant4/Configure:
	@echo "downloading geant4"
	@wget -O - http://geant4.cern.ch/support/source/geant4.9.3.tar.gz | tar xz
	@mv geant4.9.3 geant4
	@mkdir -p share/geant4/data
	@cd share/geant4/data; wget -O - http://geant4.cern.ch/support/source/G4EMLOW.6.9.tar.gz | tar xz
	@cd share/geant4/data; wget -O - http://geant4.cern.ch/support/source/G4NDL.3.13.tar.gz | tar xz
	@cd share/geant4/data; wget -O - http://geant4.cern.ch/support/source/PhotonEvaporation.2.0.tar.gz | tar xz
	@cd share/geant4/data; wget -O - http://geant4.cern.ch/support/source/G4RadioactiveDecay.3.2.tar.gz | tar xz
	@cd share/geant4/data; wget -O - http://geant4.cern.ch/support/source/G4ABLA.3.0.tar.gz | tar xz
	@cd share/geant4/data; wget -O - http://geant4.cern.ch/support/source/RealSurface.1.0.tar.gz | tar xz

# GEANT4 build command
geant4/env.sh: CLHEP/config.log geant4/Configure
	@echo "building geant4"
	@-cd geant4; patch -Np0 < ../geant4.patch
	@cd geant4; ./Configure -build -d -e -s -D d_portable=y -D g4includes_flag=y \
	-D g4data=$(EXTDIR)/share/geant4/data -D g4clhep_base_dir=$(EXTDIR) \
	-D g4clhep_include_dir=$(EXTINCDIR) -D g4clhep_lib_dir=$(EXTLIBDIR)
	@-rm -rf geant4/env.*sh; cd geant4; ./Configure
	@cd geant4; . ./env.sh; cd source; G4INCLUDE=$(EXTDIR)/include/geant4 make includes dependencies=""
	@cp -a $(EXTDIR)/geant4/lib/*/* $(EXTLIBDIR)


# dependence for root build
root: root/config/Makefile.config

# root build command
root/config/Makefile.config:
	@echo "building root"
	@-cd root; patch -Np0 < ../root.patch
	@cd root; ./configure --incdir=$(EXTINCDIR)/root --libdir=$(EXTLIBDIR) --bindir=$(EXTBINDIR) \
	--prefix=$(EXTDIR) --etcdir=$(EXTDIR)/share/etc --enable-gsl-shared \
	--with-g4-incdir=$(EXTINCDIR)/geant4 --with-g4-libdir=$(EXTLIBDIR) \
	--with-clhep-incdir=$(EXTINCDIR); make; make install


# dependence for genfit build
genfit: include/genfit/RKTrackRep.h

# genfit build command
include/genfit/RKTrackRep.h:
	@echo "building genfit"
	@cd genfit; SCONSFLAGS="" scons
	@cp genfit/lib/* $(EXTLIBDIR)/ # copy the libraries
	@cp -r genfit/include/* $(EXTINCDIR)/ # copy the installed files


# dependence for EvtGen build
evtgen: evtgen/config.mk

# dependence for EvtGen download
evtgen/configure:
	@echo "downloading EvtGen"
	@wget -O - http://service-spi.web.cern.ch/service-spi/external/MCGenerators/distribution/evtgenlhc-9.1-src.tgz | tar xz
	@mv evtgenlhc/9.1 evtgen
	@cd evtgen; patch -Np0 < ../evtgen.patch
	@rmdir evtgenlhc

# EvtGen build command
evtgen/config.mk: evtgen/configure
	@echo "building EvtGen"
	@cd evtgen; ./configure --lcgplatform=x86_64-slc5-gcc43-opt; make
	@cd evtgen; cp lib/* $(EXTLIBDIR)/; mkdir $(EXTINCDIR)/evtgen; cp -r EvtGen* $(EXTINCDIR)/evtgen
