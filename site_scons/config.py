from SCons.Script import *
import os, platform
from distutils import sysconfig

# check for the existance of an environment variable
def CheckEnvVar(conf, var, text = None):
    if text:
        conf.Message('Checking for %s...' % text)
    else:
        conf.Message('Checking for environment variable %s...' % var)
    result = conf.env['ENV'].has_key(var)
    conf.Result(result)
    return result

# check for the existance of a tool
def CheckConfigTool(conf, tool):
    conf.Message('Checking for %s...' % tool)
    result, version = conf.TryAction('%s --version' % tool)
    conf.Result(result)
    return result

# check for the existance a file
def CheckFile(conf, dir, text = None):
    if text:
        conf.Message('Checking for %s...' % text)
    else:
        conf.Message('Checking for directory %s...' % dir)
    if (conf.env.FindFile(dir, '.') == None):
        result = 0
    else:
        result = 1
    conf.Result(result)
    return result


# check the Belle II environment
def configure_belle2(conf):
    
    # Belle II environment setup
    if not conf.CheckEnvVar('BELLE2_TOOLS', 'Belle II environment setup'):
        print 'Belle II software environment is not set up.'
        print '-> Source "setup_belle2.sh" (for bash) or "setup_belle2.csh" (for csh).'
        return False

    # local Belle II release setup
    if not conf.CheckEnvVar('BELLE2_LOCAL_DIR', 'local release setup'):
        print 'local release is not set up.'
        print '-> Execute "setuprel" in your local release directory.'
        return False

    return True


# configure the system packages
def configure_system(conf):
    
    # python
    python_version = platform.python_version_tuple()
    conf.env['PYTHON_LIBS'] = ['python%s.%s' % (python_version[0], python_version[1])]

    # xml
    if not conf.CheckConfigTool('xml2-config'):
        print 'XML configuration tool missing'
        print '-> install the libxml2 development package'
        return False
        
    conf.env.ParseConfig('xml2-config --cflags')
    xml_env = Environment(ENV = os.environ)
    xml_env.ParseConfig('xml2-config --libs')
    conf.env['XML_LIBS'] = xml_env['LIBS']

    return True


# check the system packages
# (this doesn't work for some reason)
def check_system(conf):
    
    if not conf.CheckLib('xml'):
        print 'XML library missing'
        print '-> install the libxml2 package'
        return False
    if not conf.CheckCHeader('libxml/xmlversion.h'):
        print 'XML header files missing'
        print '-> install the libxml2 development package'
        return False

    return True


# configure the external packages
def configure_externals(conf):
    
    # configure externals only after they have been built
    if BUILD_TARGETS == ['externals']:
        return True

    # boost
    conf.env.Append(CPPPATH = os.path.join(conf.env['EXTINCDIR'], 'boost'))

    # CLHEP
    conf.env.Append(CPPPATH = os.path.join(conf.env['EXTINCDIR'], 'CLHEP'))

    # geant4
    conf.env.Append(CPPPATH = os.path.join(conf.env['EXTINCDIR'], 'geant4'))
    conf.env['GEANT4_LIBS'] = ['G4digits_hits', 'G4error_propagation', 'G4event',
                               'G4FR', 'G4geometry', 'G4global',
                               'G4graphics_reps', 'G4intercoms', 'G4interfaces',
                               'G4materials', 'G4modeling', 'G4parmodels',
                               'G4particles', 'G4physicslists',
                               'G4processes', 'G4RayTracer', 'G4readout',
                               'G4run', 'G4track', 'G4tracking', 'G4Tree',
                               'G4visHepRep', 'G4vis_management', 'G4visXXX',
                               'G4VRML']

    # root
    conf.env.Append(CPPPATH = os.path.join(conf.env['EXTINCDIR'], 'root'))

    conf.env['ROOT_LIBS'] = conf.env['ROOT_GLIBS'] = []
    if conf.CheckConfigTool('root-config'):
        root_env = Environment(ENV = os.environ)
        root_env.ParseConfig('root-config --libs')
        conf.env['ROOT_LIBS'] = root_env['LIBS']
        root_env.ParseConfig('root-config --glibs')
        conf.env['ROOT_GLIBS'] = root_env['LIBS']
    else:
        print 'root configuration tool missing'
        print '-> create it with the command "scons externals"'
        return False

    return True


# check the external packages
def check_externals(conf):
    
    # boost
    if not conf.CheckFile('externals/include/boost/version.hpp', 'boost'):
        if not conf.CheckFile('externals/boost/bootstrap.sh', 'boost source code'):
            print 'boost is missing'
            print '-> add and build the externals package'
            return False
        if BUILD_TARGETS[0] not in ['externals', 'externals.boost']:
            print 'boost is not built'
            print '-> build it with the command "scons externals"'
            return False

    # CLHEP
    if not conf.CheckFile('externals/include/CLHEP/ClhepVersion.h', 'CLHEP'):
        if not conf.CheckFile('externals/Makefile', 'externals package'):
            print 'CLHEP is missing'
            print '-> add and build the externals package'
            return False
        if BUILD_TARGETS[0] not in ['externals', 'externals.CLHEP']:
            print 'CLHEP is not built'
            print '-> build it with the command "scons externals"'
            return False

    # geant4
    if not conf.CheckFile('externals/include/geant4/G4Version.hh', 'geant4'):
        if not conf.CheckFile('externals/Makefile', 'externals package'):
            print 'geant4 is missing'
            print '-> add and build the externals package'
            return False
        if BUILD_TARGETS[0] not in ['externals', 'externals.geant4']:
            print 'geant4 is not built'
            print '-> build it with the command "scons externals"'
            return False

    # root
    root_target = (len(BUILD_TARGETS) == 1) and (BUILD_TARGETS[0] in ['externals', 'externals.root'])
    if not conf.CheckFile('externals/include/root/RVersion.h', 'root'):
        if not conf.CheckFile('externals/root/configure', 'root source code'):
            print 'root is missing'
            print '-> add and build the externals package'
            return False
        if not root_target:
            print 'root is not built'
            print '-> build it with the command "scons externals"'
            return False

    return True



# configure the environment
def configure(env):

    conf = Configure(env, custom_tests = {'CheckEnvVar' : CheckEnvVar,
                                          'CheckConfigTool' : CheckConfigTool,
                                          'CheckFile' : CheckFile})

    if (not configure_belle2(conf)) or (not configure_system(conf)) or (not configure_externals(conf)):
        return False

    env = conf.Finish()

    return True
