#!/usr/bin/env python
# -*- coding: utf-8 -*-

from SCons.Script import *
import os
import platform
from distutils import sysconfig
import subprocess


def CheckEnvVar(conf, var, text=None):
    """check for the existance of an environment variable"""

    if text:
        conf.Message('Checking for %s...' % text)
    else:
        conf.Message('Checking for environment variable %s...' % var)
    result = var in conf.env['ENV']
    conf.Result(result)
    return result


def CheckConfigTool(conf, tool):
    """check for the existance of a tool"""

    conf.Message('Checking for %s...' % tool)
    (result, version) = conf.TryAction('%s --version' % tool)
    conf.Result(result)
    return result


def CheckPackage(conf, package, text=None):
    """check for the existance of a package via the pkg-config tool"""

    if not text:
        text = package
    conf.Message('Checking for %s...' % text)
    (result, output) = conf.TryAction('pkg-config --exists %s' % package)
    conf.Result(result)
    return result


def CheckFile(conf, dir, text=None):
    """check for the existance a file"""

    if text:
        conf.Message('Checking for %s...' % text)
    else:
        conf.Message('Checking for directory %s...' % dir)
    if conf.env.FindFile(dir, '.') == None:
        result = 0
    else:
        result = 1
    conf.Result(result)
    return result


def CheckLibrary(conf, lib):
    """check for the loading of a library"""

    conf.Message('Checking for %s...' % lib)
    process = subprocess.Popen(['ldd', lib], stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    output = process.communicate()[0]
    if process.returncode == 0 and not 'not found' in output:
        result = 1
    else:
        result = 0
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
    if not conf.CheckEnvVar('BELLE2_ANALYSIS_DIR', 'analysis setup') \
        and not conf.CheckEnvVar('BELLE2_LOCAL_DIR', 'local release setup'):
        print 'analysis or local release is not set up.'
        print '-> Execute "setupana" or "setuprel" in your local analysis or release directory, respectively.'
        return False

    return True


def configure_system(conf):
    """configure the system packages"""

    # python
    python_version = platform.python_version_tuple()
    conf.env['PYTHON_LIBS'] = ['python%s.%s' % (python_version[0],
                               python_version[1])]

    # xml
    if not conf.CheckConfigTool('xml2-config'):
        print 'XML configuration tool missing'
        print '-> install the libxml2 development package'
        return False

    conf.env.ParseConfig('xml2-config --cflags')
    xml_env = Environment(ENV=os.environ)
    xml_env.ParseConfig('xml2-config --libs')
    conf.env['XML_LIBS'] = xml_env['LIBS']

    # TEve
    conf.env['HAS_TEVE'] = False
    conf.env['TEVE_LIBS'] = []
    if conf.CheckLibrary(os.path.join(os.environ['ROOTSYS'], 'lib', 'libEve.so'
                         )):
        conf.env['HAS_TEVE'] = True
        conf.env['TEVE_LIBS'] = ['Gui', 'Eve', 'Ged', 'RGL', 'TreePlayer']

    # sqlite3
    conf.env['HAS_SQLITE'] = False
    conf.env['SQLITE_LIBS'] = []
    if conf.CheckLibWithHeader('sqlite3', 'sqlite3.h', 'C',
                               'sqlite3_open_v2(":memory:",0,SQLITE_OPEN_READONLY,0);'
                               ):
        conf.env['HAS_SQLITE'] = True
        conf.env.Append(CPPDEFINES='-DHAS_SQLITE')
        sqlite_env = Environment(ENV=os.environ)
        sqlite_env.ParseConfig('pkg-config sqlite3 --libs')
        conf.env['SQLITE_LIBS'] = sqlite_env['LIBS']

    # valgrind
    conf.env['HAS_CALLGRIND'] = False
    if conf.CheckHeader('valgrind/callgrind.h'):
        conf.env['HAS_CALLGRIND'] = True
        conf.env.Append(CPPDEFINES='-DHAS_CALLGRIND')

    return True


def configure_externals(conf):
    """configure the external packages"""

    try:
        extdir = conf.env['EXTDIR']
        sys.path[:0] = [os.environ['BELLE2_TOOLS'], extdir]
        from externals import config_externals
        return config_externals(conf)
    except:

        print 'Configuration of externals failed.'
        return False

    return True


def configure(env):
    """configure the environment"""

    conf = Configure(env, custom_tests={
        'CheckEnvVar': CheckEnvVar,
        'CheckConfigTool': CheckConfigTool,
        'CheckPackage': CheckPackage,
        'CheckFile': CheckFile,
        'CheckLibrary': CheckLibrary,
        })

    if not configure_belle2(conf) or not configure_system(conf) \
        or not configure_externals(conf):
        return False

    env = conf.Finish()

    return True


