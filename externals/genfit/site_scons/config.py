from SCons.Script import *
import os, platform
from distutils import sysconfig


def CheckEnvVar(conf, var, text = None):
    """check for the existance of an environment variable"""

    if text:
        conf.Message('Checking for %s...' % text)
    else:
        conf.Message('Checking for environment variable %s...' % var)
    result = conf.env['ENV'].has_key(var)
    conf.Result(result)
    return result


def CheckConfigTool(conf, tool):
    """check for the existance of a tool"""

    conf.Message('Checking for %s...' % tool)
    result, version = conf.TryAction('%s --version' % tool)
    conf.Result(result)
    return result


def CheckPackage(conf, package, text = None):
    """check for the existance of a package via the pkg-config tool"""

    if not text:
        text = package
    conf.Message('Checking for %s...' % text)
    result, output = conf.TryAction('pkg-config --exists %s' % package)
    conf.Result(result)
    return result


def CheckFile(conf, dir, text = None):
    """check for the existance a file"""

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


def configure_belle2(conf):
    """check the Belle II environment"""

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


def configure_externals(conf):
    """configure the external packages"""

    # root
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


def check_externals(conf):
    """check the external packages"""

    # root
    if not conf.CheckFile('../include/root/RVersion.h', 'root'):
        if not conf.CheckFile('../root/configure', 'root source code'):
            print 'root is missing'
            print '-> add and build the externals package'
            return False
        else:
            print 'root is not built'
            print '-> build it with the command "scons externals"'
            return False

    return True



def configure(env):
    """configure the environment"""

    conf = Configure(env, custom_tests = {'CheckEnvVar' : CheckEnvVar,
                                          'CheckConfigTool' : CheckConfigTool,
                                          'CheckPackage' : CheckPackage,
                                          'CheckFile' : CheckFile})

    if (not configure_belle2(conf)) or (not configure_externals(conf)):
        return False

    env = conf.Finish()

    return True
