import os

Import('env')

# prepare directory names
belle2_local_dir = os.environ.get('BELLE2_LOCAL_DIR')
ext_dir = os.path.join(belle2_local_dir, env.subst('$EXTDIR').replace('#', ''))
ext_inc_dir = os.path.join(belle2_local_dir, env.subst('$EXTINCDIR').replace('#', ''))
ext_lib_dir = os.path.join(belle2_local_dir, env.subst('$EXTLIBDIR').replace('#', ''))
ext_bin_dir = os.path.join(belle2_local_dir, env.subst('$EXTBINDIR').replace('#', ''))


# boost
boost_builder = Builder(action = """
cd externals/boost; \
./bootstrap.sh --includedir=%s --libdir=%s; \
./bjam install
""" % (ext_inc_dir, ext_lib_dir))
env['BUILDERS']['make_boost'] = boost_builder

boost = env.make_boost('boost/project-config.jam', 'boost/bootstrap.sh')
env.Alias('externals.boost', boost)
env.Alias('externals', boost)


# CLHEP
clhep_version = open('CLHEP.version').readline().strip()
clhep_downloader = Builder(action = """
cd externals; \
wget -O - http://proj-clhep.web.cern.ch/proj-clhep/DISTRIBUTION/tarFiles/clhep-%s.tgz | tar xz; \
rm -rf CLHEP; \
mv %s/CLHEP .; \
rmdir %s
""" % (clhep_version, clhep_version, clhep_version))
env['BUILDERS']['download_clhep'] = clhep_downloader

clhep_builder = Builder(action = """
cd externals/CLHEP; \
./configure --prefix=%s --includedir=%s --libdir=%s --bindir=%s; \
make; \
make install
""" % (ext_dir, ext_inc_dir, ext_lib_dir, ext_bin_dir))
env['BUILDERS']['make_clhep'] = clhep_builder

clhep_source = env.download_clhep('CLHEP/configure', 'CLHEP.version')
clhep = env.make_clhep('CLHEP/config.log', clhep_source)
env.Alias('externals.clhep', clhep)
env.Alias('externals', clhep)


# GEANT4
geant4_version = open('geant4.version').readline().strip()
geant4_downloader = Builder(action = """
cd externals; \
wget -O - http://geant4.cern.ch/support/source/geant%s.tar.gz | tar xz; \
ls -la; \
rm -rf geant4; \
mv geant%s geant4
mkdir -p externals/share/geant4/data
cd externals/share/geant4/data; \
wget -O - http://geant4.cern.ch/support/source/G4EMLOW.6.9.tar.gz | tar xz; \
wget -O - http://geant4.cern.ch/support/source/G4NDL.3.13.tar.gz | tar xz; \
wget -O - http://geant4.cern.ch/support/source/PhotonEvaporation.2.0.tar.gz | tar xz; \
wget -O - http://geant4.cern.ch/support/source/G4RadioactiveDecay.3.2.tar.gz | tar xz; \
wget -O - http://geant4.cern.ch/support/source/G4ABLA.3.0.tar.gz | tar xz; \
wget -O - http://geant4.cern.ch/support/source/RealSurface.1.0.tar.gz | tar xz
""" % (geant4_version, geant4_version))
env['BUILDERS']['download_geant4'] = geant4_downloader

geant4_builder = Builder(action = """
cd externals/geant4; \
rm -f Configure.new; \
cat Configure | sed "s/g4granular='n'/g4granular='y'/g" | sed "s/g4granular=n/g4granular=y/g" > Configure.new; \
chmod a+x Configure.new
cd externals/geant4; \
./Configure.new -build -d -e -s -D d_portable=y -D g4includes_flag=y \
                -D g4data=%s/share/geant4/data -D g4clhep_base_dir=%s \
	        -D g4clhep_include_dir=%s -D g4clhep_lib_dir=%s
cd externals/geant4; \
rm -rf env.*sh; \
./Configure; \
. ./env.sh; \
cd source; \
G4INCLUDE=%s/geant4 make includes dependencies=""
cp -a externals/geant4/lib/*/* %s
""" % (ext_dir, ext_dir, ext_inc_dir, ext_lib_dir, ext_inc_dir, ext_lib_dir))
env['BUILDERS']['make_geant4'] = geant4_builder

geant4_source = env.download_geant4('geant4/Configure', 'geant4.version')
geant4 = env.make_geant4('geant4/env.sh', [geant4_source, clhep])
env.Alias('externals.geant4', geant4)
env.Alias('externals', geant4)


# root
root_builder = Builder(action = """
cd externals/root; \
./configure --incdir=%s/root --libdir=%s --bindir=%s \
            --prefix=%s --etcdir=%s/share/etc --enable-gsl-shared \
	    --with-g4-incdir=%s/geant4 --with-g4-libdir=%s \
	    --with-clhep-incdir=%s; \
make; \
make install
""" % (ext_inc_dir, ext_lib_dir, ext_bin_dir, ext_dir, ext_dir, ext_inc_dir, ext_lib_dir, ext_inc_dir))
env['BUILDERS']['make_root'] = root_builder

root = env.make_root('root/config/Makefile.config', ['root/configure', geant4])
env.Alias('externals.root', root)
env.Alias('externals', root)


env['CONTINUE'] = False
Return('env')
