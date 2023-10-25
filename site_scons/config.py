#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from SCons.Script import Configure, Environment
import os
import sys


def CheckEnvVar(conf, var, text=None):
    """check for the existance of an environment variable"""

    if text:
        conf.Message(f'Checking for {text}...')
    else:
        conf.Message(f'Checking for environment variable {var}...')
    result = var in conf.env['ENV']
    conf.Result(result)
    return result


def CheckConfigTool(conf, tool):
    """check for the existance of a tool"""

    conf.Message(f'Checking for {tool}...')
    (result, version) = conf.TryAction(f'{tool} --version')
    conf.Result(result)
    return result


def CheckPackage(conf, package, text=None):
    """check for the existance of a package via the pkg-config tool"""

    if not text:
        text = package
    conf.Message(f'Checking for {text}...')
    (result, output) = conf.TryAction(f'pkg-config --exists {package}')
    conf.Result(result)
    return result


def CheckFile(conf, dir, text=None):
    """check for the existance a file"""

    if text:
        conf.Message(f'Checking for {text}...')
    else:
        conf.Message(f'Checking for directory {dir}...')
    if conf.env.FindFile(dir, '.') is None:
        result = 0
    else:
        result = 1
    conf.Result(result)
    return result


def configure_belle2(conf):
    """check the Belle II environment"""

    # Belle II environment setup
    if not conf.CheckEnvVar('BELLE2_TOOLS', 'Belle II environment setup'):
        print('Belle II software environment is not set up.')
        print('-> Source "setup_belle2" from the tools/ directory.')
        return False

    # local Belle II release setup
    if not conf.CheckEnvVar('BELLE2_ANALYSIS_DIR', 'analysis setup') \
            and not conf.CheckEnvVar('BELLE2_LOCAL_DIR', 'local release setup'):
        print('analysis or local release is not set up.')
        print('-> Execute "b2setup" in your local analysis or release directory.')
        return False

    return True


def configure_system(conf):
    """configure the system packages"""

    # xml (in externals)
    conf.env.ParseConfig('xml2-config --cflags')
    xml_env = Environment(ENV=os.environ)
    xml_env.ParseConfig('xml2-config --libs')
    conf.env['XML_LIBS'] = xml_env['LIBS']

    # TEve
    conf.env['HAS_TEVE'] = False
    conf.env['TEVE_LIBS'] = []
    if conf.CheckLibWithHeader("Eve", "TEveViewer.h", language="c++", autoadd=0, call="TEveViewer();"):
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

    # check for OpenMP
    conf.env['HAS_OPENMP'] = False
    if conf.CheckHeader('omp.h', language="C++"):
        conf.env['HAS_OPENMP'] = True
        conf.env.Append(CPPDEFINES='-DHAS_OPENMP')

    # graphviz
    conf.env['HAS_DOT'] = False
    if conf.CheckProg('dot'):
        conf.env['HAS_DOT'] = True

    return True


def configure_externals(conf):
    """configure the external packages"""

    try:
        extdir = conf.env['EXTDIR']
        sys.path[:0] = [os.environ['BELLE2_TOOLS'], extdir]
        from externals import config_externals
        return config_externals(conf)
    except Exception as e:
        print('Configuration of externals failed:', e)
        return False

    return True


def configure(env):
    """configure the environment"""

    conf = Configure(env, custom_tests={
        'CheckEnvVar': CheckEnvVar,
        'CheckConfigTool': CheckConfigTool,
        'CheckPackage': CheckPackage,
        'CheckFile': CheckFile,
    })

    if not configure_belle2(conf) or not configure_externals(conf) \
            or not configure_system(conf):
        return False

    env = conf.Finish()

    return True
