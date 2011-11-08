#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from SCons.Script import *


def define_aliases(
    env,
    target,
    dir_name,
    extension=None,
    ):

    parent_dir = dir_name
    while len(parent_dir) > 0:
        env.Alias(parent_dir, target)
        if extension:
            env.Alias(parent_dir + '.' + extension, target)
        parent_dir = os.path.split(parent_dir)[0]
    if extension:
        env.Alias(extension, target)


def get_files(path_name, release_dir):
    if release_dir:
        save_dir = os.getcwd()
        os.chdir(release_dir)
        result = Glob(path_name)
        os.chdir(save_dir)
        return result
    else:
        return Glob(path_name)


def real_path(path_name, release_dir):
    if release_dir:
        return os.path.join(release_dir, path_name)
    else:
        return path_name


def process_dir(
    parent_env,
    dir_name,
    is_module_dir,
    release_dir,
    ):

    # determine library name
    lib_name = dir_name.replace(os.sep, '_')

    # get list of header and linkdef files
    header_files = get_files(os.path.join(dir_name, '*.h'), release_dir) \
        + get_files(os.path.join(dir_name, 'include', '*.h'), release_dir)
    linkdef_files = []
    for header_file in header_files:
        if str(header_file).lower().endswith('linkdef.h'):
            linkdef_files.append(header_file)
            header_files.remove(header_file)

    # get list of source files
    src_nodes = get_files(os.path.join(dir_name, '*.cc'), release_dir) \
        + get_files(os.path.join(dir_name, 'src', '*.cc'), release_dir) \
        + get_files(os.path.join(dir_name, '*.f'), release_dir) \
        + get_files(os.path.join(dir_name, 'src', '*.f'), release_dir) \
        + get_files(os.path.join(dir_name, '*.F'), release_dir) \
        + get_files(os.path.join(dir_name, 'src', '*.F'), release_dir)
    src_files = [os.path.join(parent_env['BUILDDIR'], str(node)) for node in
                 src_nodes]

    # get list of test files
    test_files = [os.path.join(parent_env['BUILDDIR'], str(node)) for node in
                  get_files(os.path.join(dir_name, 'tests', '*.cc'),
                  release_dir)]

    # get list of script files
    script_files = get_files(os.path.join(dir_name, 'scripts', '*'),
                             release_dir)

    # get list of data files
    data_files = get_files(os.path.join(dir_name, 'data', '*'), release_dir)

    # create environment for directory
    env = parent_env.Clone()
    env['HEADER_FILES'] = header_files
    env['LINKDEF_FILES'] = linkdef_files
    env['SRC_FILES'] = src_files
    env['TEST_FILES'] = test_files
    env['TEST_LIBS'] = []
    env['SCRIPT_FILES'] = script_files
    env['DATA_FILES'] = data_files

    # include SConscript file if it exists
    if env.Dictionary().has_key('SUBLIB'):
        del env.Dictionary()['SUBLIB']
    if env.Dictionary().has_key('PYTHON_MODULE'):
        del env.Dictionary()['PYTHON_MODULE']
    sconscript_name = real_path(os.path.join(dir_name, 'SConscript'),
                                release_dir)
    if os.path.isfile(sconscript_name):
        result = SConscript(sconscript_name, exports='env')

        # use the new environment if it was updated by the SConscript file
        if isinstance(result, Environment):
            env = result

            # don't continue with the default build process if the SConscript file requests this
            if env.Dictionary().has_key('CONTINUE') and env['CONTINUE'] \
                == False:
                return

    # install header files in the include directory
    includes = env.Install(os.path.join(env['INCDIR'], dir_name),
                           env['HEADER_FILES'])
    define_aliases(env, includes, dir_name, 'include')

    # install script files in the library directory
    scripts = env.Install(env['LIBDIR'], env['SCRIPT_FILES'])
    Local(scripts)
    define_aliases(env, scripts, dir_name, 'scripts')

    # install data files in the data directory
    data = env.Install(os.path.join(env['DATADIR'], dir_name), env['DATA_FILES'
                       ])
    Local(data)
    define_aliases(env, data, dir_name, 'data')

    # remember tests defined in this directory
    local_test_files = env['TEST_FILES']

    # loop over subdirs
    entries = os.listdir(real_path(dir_name, release_dir))
    for entry in entries:
        if entry.find('.') == -1 \
            and not os.path.isfile(real_path(os.path.join(dir_name, entry),
                                   release_dir)) and not entry in [
            'include',
            'src',
            'tools',
            'tests',
            'scripts',
            'data',
            'doc',
            'examples',
            ]:
            process_dir(env, os.path.join(dir_name, entry), entry == 'modules'
                        or is_module_dir, release_dir)

    # check whether we have to create a new library
    if env.Dictionary().has_key('MODULE') and env['MODULE'] == True:
        is_module_dir = True
    if dir_name == env['PACKAGE'] or env.Dictionary().has_key('SUBLIB') \
        and env['SUBLIB'] == True or env.Dictionary().has_key('PYTHON_MODULE') \
        and env['PYTHON_MODULE'] == True or is_module_dir \
        and not (env.Dictionary().has_key('SUBLIB') and env['SUBLIB']
                 == False):

        # generate dictionaries
        dict_files = []
        for linkdef_file in env['LINKDEF_FILES']:
            dict_filename = str(linkdef_file).replace(os.sep, '_')[:-9] \
                + 'Dict.cc'
            dict_file = env.RootDict(os.path.join(env['BUILDDIR'],
                                     dict_filename), linkdef_file)
            # add current directory to include path for dictionary compilation
            dict_files.append(env.SharedObject(dict_file, CPPPATH=['.']
                              + env['CPPPATH']))

        # build a shared library with all source and dictionary files
        if len(env['SRC_FILES']) > 0:
            lib_dir_name = env['LIBDIR']
            if is_module_dir:
                lib_dir_name = env['MODDIR']
                if os.path.basename(dir_name) != 'modules':
                    lib_name = os.path.basename(dir_name)

            lib = env.SharedLibrary(os.path.join(lib_dir_name, lib_name),
                                    [env['SRC_FILES'], dict_files])
            reg_map = env.RegMap(os.path.join(lib_dir_name,
                                 env.subst('$SHLIBPREFIX') + lib_name + '.map'
                                 ), env['SRC_FILES'])
            Local([lib, reg_map])
            env.Alias(lib_name, [lib, reg_map])
            if is_module_dir:
                define_aliases(env, [lib, reg_map], dir_name, 'modules')
            else:
                define_aliases(env, [lib, reg_map], dir_name, 'lib')
            if env.Dictionary().has_key('PYTHON_MODULE') \
                and env['PYTHON_MODULE'] == True:
                pymod = env.InstallAs(os.path.join(env['LIBDIR'],
                                      os.path.basename(dir_name) + 'module'
                                      + env.subst('$SHLIBSUFFIX')), lib)
                Local(pymod)
                define_aliases(env, pymod, dir_name, 'lib')
    else:

        # add linkdef, and source files to parent environment if we are in a normal sub-directory
        parent_env['LINKDEF_FILES'] += env['LINKDEF_FILES']
        parent_env['SRC_FILES'] += env['SRC_FILES']

    # setup environment for building executables, include SConscript if it exists
    save_env = env.Clone()
    env['TOOLS_FILES'] = get_files(os.path.join(dir_name, 'tools', '*.cc'),
                                   release_dir)
    sconscript_name = real_path(os.path.join(dir_name, 'tools', 'SConscript'),
                                release_dir)
    if os.path.isfile(sconscript_name):
        result = SConscript(sconscript_name, exports='env')
        if isinstance(result, Environment):
            env = result

    # build a binary for each source file in the tools directory
    for bin_file in env['TOOLS_FILES']:
        bin_filename = os.path.splitext(os.path.basename(str(bin_file)))[0]
        bin_env = env.Clone()
        if bin_env['TOOLS_LIBS'].has_key(bin_filename):
            bin_env['LIBS'] = Flatten([env.subst(str(lib)).split() for lib in
                                      Flatten(bin_env['TOOLS_LIBS'
                                      ][bin_filename])])
        if bin_env['TOOLS_LIBPATH'].has_key(bin_filename):
            bin_env['LIBPATH'] = bin_env['TOOLS_LIBPATH'][bin_filename]
        tool = bin_env.Program(os.path.join(bin_env['BINDIR'], bin_filename),
                               os.path.join(bin_env['BUILDDIR'],
                               str(bin_file)))
        env.Alias(os.path.join(dir_name, 'tools', bin_filename), tool)
        env.Alias(os.path.join(dir_name, 'tools'), tool)
        env.Alias(os.path.join(dir_name, bin_filename), tool)
        define_aliases(env, tool, dir_name, 'bin')

    # restore original environment
    env = save_env

    # check whether we have to create a new test executable
    if len(local_test_files) > 0:
        test_filename = 'test_' + lib_name
        test_env = env.Clone()
        sconscript_name = real_path(os.path.join(dir_name, 'tests',
                                    'SConscript'), release_dir)
        if os.path.isfile(sconscript_name):
            result = SConscript(sconscript_name, exports='env')
            if isinstance(result, Environment):
                test_env = result
        test_env.Append(LIBS=['gtest', lib_name])
        env.AppendUnique(TEST_LIBS=test_env['LIBS'])
        env['TEST_FILES'] = test_env.SharedObject(local_test_files)
        test = test_env.Program(os.path.join(test_env['BINDIR'],
                                test_filename), env['TEST_FILES'])
        env.Alias(os.path.join(dir_name, 'tests', test_filename), test)
        env.Alias(os.path.join(dir_name, 'tests'), test)
        env.Alias(os.path.join(dir_name, test_filename), test)
        define_aliases(env, test, dir_name, 'tests')

    # add test files and libs to parent environment
    parent_env.AppendUnique(TEST_LIBS=env['TEST_LIBS'])
    parent_env['TEST_FILES'] += env['TEST_FILES']


def generate(env):
    env.AddMethod(process_dir, 'ProcessDirectory')


def exists(env):
    return True


