#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import stat
from SCons.Script import Environment, File, Flatten, Glob, SConscript


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

        next_parent_dir = os.path.split(parent_dir)[0]
        # check split actually does something here
        if next_parent_dir == parent_dir:
            break
        parent_dir = next_parent_dir

    if extension:
        env.Alias(extension, target)


def get_files(path_name):
    result = Glob(path_name)
    files = [f for f in result if not os.path.isdir(str(f))]
    return files


def get_python_files_recursive(topdir_path):
    python_file_nodes = []

    for (dir_path, dir_names, file_names) in os.walk(topdir_path):
        for file_name in file_names:
            if file_name.endswith('.py'):
                file_path = os.path.join(dir_path, file_name)
                file_node = File(file_path)
                python_file_nodes.append(file_node)

    return python_file_nodes


def process_dir(
    parent_env,
    dir_name,
    is_module_dir
):

    # remove leading ./
    if dir_name.startswith('./'):
        dir_name = dir_name[2:]

    # determine library name
    if dir_name == '.':
        lib_name = parent_env['PACKAGE']
    else:
        lib_name = dir_name.replace(os.sep, '_')

    # get list of header and linkdef files
    header_files = get_files(os.path.join(dir_name, '*.h'))
    if dir_name != '.':
        header_files += get_files(os.path.join(dir_name, 'include', '*.h'))
    linkdef_files = []
    for header_file in header_files:
        if str(header_file).lower().endswith('linkdef.h'):
            linkdef_files.append(header_file)
            header_files.remove(header_file)

    # get list of source files
    src_nodes = get_files(os.path.join(dir_name, '*.cc')) \
        + get_files(os.path.join(dir_name, 'src', '*.cc')) \
        + get_files(os.path.join(dir_name, '*.c')) \
        + get_files(os.path.join(dir_name, 'src', '*.c')) \
        + get_files(os.path.join(dir_name, '*.f')) \
        + get_files(os.path.join(dir_name, 'src', '*.f')) \
        + get_files(os.path.join(dir_name, '*.F')) \
        + get_files(os.path.join(dir_name, 'src', '*.F'))
    src_files = [os.path.join(parent_env['BUILDDIR'], str(node)) for node in
                 src_nodes]

    # get list of test files
    test_files = [os.path.join(parent_env['BUILDDIR'], str(node)) for node in
                  get_files(os.path.join(dir_name, 'tests', '*.cc'))]

    # get list of script files
    script_files = get_python_files_recursive(os.path.join(dir_name, 'scripts'
                                                           ))

    # get list of executable script files
    executable_files = []
    executable_mode = stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH
    for tools_file in get_files(os.path.join(dir_name, 'tools', '*')):
        executable_file = str(tools_file)
        if os.stat(executable_file).st_mode & executable_mode \
                == executable_mode:
            executable_files.append(tools_file)

    # get list of data files
    data_files = get_files(os.path.join(dir_name, 'data', '*'))

    # create environment for directory
    env = parent_env.Clone()
    env['HEADER_FILES'] = header_files
    env['LINKDEF_FILES'] = linkdef_files
    env['SRC_FILES'] = src_files
    env['TEST_FILES'] = test_files
    env['TEST_LIBS'] = []
    env['SCRIPT_FILES'] = script_files
    env['EXECUTABLE_FILES'] = executable_files
    env['DATA_FILES'] = data_files
    env['DATAOBJECT_LIB'] = []
    env['DATAOBJECT_LIBS'] = []

    # clean up some environment variables that should not be inherited from the parent environment
    if 'SUBLIB' in env.Dictionary():
        del env.Dictionary()['SUBLIB']
    if 'PYTHON_MODULE' in env.Dictionary():
        del env.Dictionary()['PYTHON_MODULE']
    if 'LIBS' in env.Dictionary():
        del env.Dictionary()['LIBS']

    if dir_name in env.get('DISABLE_COMPILER_WARNINGS', []):
        env.AppendUnique(CXXFLAGS=['-w'], CCFLAGS=['-w'], FORTRANFLAGS=['-w'], LINKFLAGS=['-w'])

    # link dataobjects to analysis modules
    if dir_name == '.':
        env.Append(LIBS=['dataobjects'])

    # include SConscript file if it exists
    sconscript_name = os.path.join(dir_name, 'SConscript')
    if os.path.isfile(sconscript_name):
        result = SConscript(sconscript_name, exports='env')

        # use the new environment if it was updated by the SConscript file
        if isinstance(result, Environment):
            env = result

            # don't continue with the default build process if the SConscript file requests this
            if not env.Dictionary().get('CONTINUE', True):
                return

    # Add additional sources.
    if 'ADDITIONAL_SOURCES' in env.Dictionary():
        additional_src_nodes = []
        for source in env.Dictionary()['ADDITIONAL_SOURCES']:
            additional_src_nodes += get_files(os.path.join(dir_name, source))
        additional_src_files = [
            os.path.join(parent_env['BUILDDIR'], str(node)) for node in
            additional_src_nodes]
        if (len(additional_src_files) > 0):
            src_files.append(additional_src_files)
        env['SRC_FILES'] = src_files

    # install header files in the include directory
    includes = env.Install(os.path.join(env['INCDIR'], dir_name),
                           env['HEADER_FILES'])
    define_aliases(env, includes, dir_name, 'include')

    # install script files in the library directory
    script_targets = []
    for script_file_node in env['SCRIPT_FILES']:
        script_file_path = str(script_file_node)
        script_dir = os.path.dirname(script_file_path)

        destination_reldir = os.path.relpath(script_dir,
                                             os.path.join(dir_name, 'scripts'))

        if not destination_reldir:
            continue
        destination_dir = os.path.join(env['LIBDIR'], destination_reldir)
        script_target = env.Install(destination_dir, script_file_node)
        script_targets.append(script_target)

    define_aliases(env, script_targets, dir_name, 'scripts')

    # install executable script files in the bin directory
    executables = env.Install(env['BINDIR'], env['EXECUTABLE_FILES'])
    define_aliases(env, executables, dir_name, 'tools')

    # install data files in the data directory
    data = env.Install(os.path.join(env['DATADIR'], dir_name), env['DATA_FILES'
                                                                   ])
    define_aliases(env, data, dir_name, 'data')

    # remember tests defined in this directory
    local_test_files = env['TEST_FILES']

    # loop over subdirs
    entries = os.listdir(dir_name)
    exclude_dirs = set()
    if (os.path.exists(os.path.join(dir_name, '.excluded_directories'))):
        f = open('.excluded_directories', 'r')
        for line in f.readlines():
            exclude_dirs.add(line.rstrip('\n'))
        f.close()
        print(f'Excluded directories: {exclude_dirs}')
    for entry in entries:
        if entry in exclude_dirs:
            continue
        if entry.find('.') == -1 \
            and not os.path.isfile(os.path.join(dir_name, entry)) and entry not in [
            'include',
            'src',
            'tools',
            'tests',
            'scripts',
            'data',
            'doc',
            'examples',
            'modules',
        ]:
            if dir_name == '.' and entry in [
                'build',
                'include',
                'lib',
                'bin',
                'modules',
                'data',
                'site_scons',
            ]:
                continue
            process_dir(env, os.path.join(dir_name, entry), is_module_dir and dir_name != '.')

    # determine whether we are in a special directory
    is_package_dir = dir_name == env['PACKAGE']
    is_sublib_dir = env.Dictionary().get('SUBLIB', False)
    is_python_module_dir = env.Dictionary().get('PYTHON_MODULE', False)
    is_dataobjects_dir = os.path.basename(dir_name) == 'dataobjects' \
        and env['PACKAGE'] != 'framework'
    if dir_name == 'dataobjects':
        is_dataobjects_dir = True
        is_module_dir = False
        lib_name = parent_env['PACKAGE']

    # check whether we have to create a new library
    if is_package_dir or is_sublib_dir or is_python_module_dir \
            or is_module_dir or is_dataobjects_dir:

        # generate dictionaries
        dict_files = []
        aux_dict_targets = []
        check_files = []
        for linkdef_file in env['LINKDEF_FILES']:
            # set the name of library generated at this stage
            # will be read by the RootDict builder
            dict_filename = str(linkdef_file).replace(os.sep, '_')[:-9] + 'Dict.cc'
            dict_file, rootmap_file, rootpcm_file = env.RootDict(os.path.join(env['BUILDDIR'], dict_filename), linkdef_file,
                                                                 ROOTCLING_ROOTMAP_LIB=lib_name)
            # add the extra cxxflags
            dict_ccflags = env["CCFLAGS"] + env["ROOTCLING_EXTRA_CCFLAGS"]
            # add current directory to include path for dictionary compilation
            dict_files.append(env.SharedObject(dict_file, CPPPATH=['.'] + env['CPPPATH'], CCFLAGS=dict_ccflags))

            # install corresponding pcm file in lib (for cling)
            aux_dict_targets.append(env.Copy(os.path.join(env['LIBDIR'], rootpcm_file.name), rootpcm_file))
            # install corresponding rootmap files to support auto-loading of libraries
            # once used via ROOT
            aux_dict_targets.append(env.Copy(os.path.join(env['LIBDIR'], rootmap_file.name), rootmap_file))

            # collect files for class version checks
            check_files.append((os.path.join(env['BUILDDIR'], str(linkdef_file).replace(os.sep, '_') + '.check'), linkdef_file))

        # build a shared library with all source and dictionary files
        if len(env['SRC_FILES']) > 0 or len(dict_files) > 0:

            # determine path of library and adjust path and name for modules
            lib_dir_name = env['LIBDIR']
            if is_module_dir:
                lib_dir_name = env['MODDIR']
                if os.path.basename(dir_name) != 'modules' and dir_name != '.':
                    lib_name = os.path.basename(dir_name)

            # update list of dataobject libraries
            if is_dataobjects_dir:
                parent_env['DATAOBJECT_LIB'] = lib_name

            # create library and map for modules
            lib = env.SharedLibrary(os.path.join(lib_dir_name, lib_name),
                                    [env['SRC_FILES'], dict_files])
            debug = env.StripDebug(lib)

            # make sure pcm and rootmap files are installed before the library
            env.Depends(lib, aux_dict_targets)

            lib_files = [lib, debug] + aux_dict_targets
            if is_module_dir:
                map_file = os.path.join(lib_dir_name, env.subst('$SHLIBPREFIX') + lib_name + '.b2modmap')
                # Adding lib_files is important to ensure we load local module
                # libraries if they are newer than those in central directory
                map_sources = env['SRC_FILES'] + lib_files

                reg_map = env.RegMap(map_file, map_sources)
                lib_files.append(reg_map)

                if env['MODULE_IO'] and env.get('HAS_DOT', False):
                    module_io = env.ModuleIo(reg_map)
                    env.Depends(module_io, [lib, reg_map])
                    env.Requires(module_io, [os.path.join('$BINDIR', 'basf2')])
                    env.Alias('module-io', module_io)

            # check class versions
            for check_filename, linkdef_file in check_files:
                env.ClassVersionCheck(check_filename, [linkdef_file, lib, debug] + env['REQUIRED_TOOLS'])

            # define build target aliases
            env.Alias(lib_name, lib_files)
            if is_module_dir:
                define_aliases(env, lib_files, dir_name, 'modules')
            else:
                define_aliases(env, lib_files, dir_name, 'lib')

            # install python module libraries with a file name that is recognized by python
            if is_python_module_dir:
                pymod = env.InstallAs(os.path.join(env['LIBDIR'], os.path.basename(dir_name) + env.subst('$SHLIBSUFFIX')), lib)
                define_aliases(env, pymod, dir_name, 'lib')
    else:

        # add linkdef, and source files to parent environment if we are in a normal sub-directory
        parent_env['LINKDEF_FILES'] += env['LINKDEF_FILES']
        parent_env['SRC_FILES'] += env['SRC_FILES']

    # add dataobject libs to parent environment
    if 'DATAOBJECT_LIB' in env.Dictionary():
        parent_env.Append(DATAOBJECT_LIBS=env['DATAOBJECT_LIB'])
    if 'DATAOBJECT_LIBS' in env.Dictionary():
        parent_env.AppendUnique(DATAOBJECT_LIBS=env['DATAOBJECT_LIBS'])

    # process modules directory last so that it is known whether the main library exists
    if os.path.isdir(os.path.join(dir_name, 'modules')):
        process_dir(env, os.path.join(dir_name, 'modules'), True)

    # setup environment for building executables, include SConscript if it exists
    save_env = env.Clone()
    env['TOOLS_FILES'] = get_files(os.path.join(dir_name, 'tools', '*.cc'))
    sconscript_name = os.path.join(dir_name, 'tools', 'SConscript')
    if os.path.isfile(sconscript_name):
        result = SConscript(sconscript_name, exports='env')
        if isinstance(result, Environment):
            env = result

    # build a binary for each source file in the tools directory
    for bin_file in env['TOOLS_FILES']:
        bin_filename = os.path.splitext(os.path.basename(str(bin_file)))[0]
        bin_env = env.Clone()
        bin_env['LIBS'] = []
        if bin_filename in bin_env['TOOLS_LIBS']:
            bin_env['LIBS'] = Flatten([env.subst(str(lib)).split() for lib in
                                       Flatten(bin_env['TOOLS_LIBS'
                                                       ][bin_filename])])
        if bin_filename in bin_env['TOOLS_LIBPATH']:
            bin_env['LIBPATH'] = bin_env['TOOLS_LIBPATH'][bin_filename]
        tool = bin_env.Program(os.path.join(bin_env['BINDIR'], bin_filename),
                               os.path.join(bin_env['BUILDDIR'],
                                            str(bin_file)))
        debug = bin_env.StripDebug(tool)
        env.Alias(os.path.join(dir_name, 'tools', bin_filename), [tool, debug])
        env.Alias(os.path.join(dir_name, 'tools'), [tool, debug])
        env.Alias(os.path.join(dir_name, bin_filename), [tool, debug])
        define_aliases(env, [tool, debug], dir_name, 'bin')

    # restore original environment
    env = save_env

    # build shared objects from the tests/*.cc files in this directory
    if len(local_test_files) > 0:
        local_test_env = env.Clone()
        sconscript_name = os.path.join(dir_name, 'tests', 'SConscript')
        if os.path.isfile(sconscript_name):
            result = SConscript(sconscript_name, exports='env')
            if isinstance(result, Environment):
                local_test_env = result
        local_test_env.PrependUnique(LIBS=['test_main'])
        local_test_env.AppendUnique(LIBS=['mdst_dbobjects', 'framework', '$ROOT_LIBS', 'gtest', 'pthread'])
        env['TEST_FILES'] = [test_file for test_file in env['TEST_FILES']
                             if test_file not in local_test_files]
        env.Prepend(TEST_FILES=local_test_env.SharedObject(local_test_files))
        env.AppendUnique(TEST_LIBS=local_test_env['LIBS'])

    # combine all tests from subdirectories to a new test executable
    if len(env['TEST_FILES']) > 0:
        test_filename = lib_name + '-unittests'
        test_env = env.Clone()
        test_env['LIBS'] = env['TEST_LIBS']
        test = test_env.Program(os.path.join(test_env['BINDIR'],
                                             test_filename), env['TEST_FILES'])
        env.StripDebug(test)
        env.Alias(os.path.join(dir_name, 'tests', test_filename), test)
        env.Alias(os.path.join(dir_name, 'tests'), test)
        env.Alias(os.path.join(dir_name, test_filename), test)
        define_aliases(env, test, dir_name, 'tests')

    # add test files and libs to parent environment
    parent_env.AppendUnique(TEST_LIBS=env['TEST_LIBS'])
    parent_env.Append(TEST_FILES=env['TEST_FILES'])


def generate(env):
    env.AddMethod(process_dir, 'ProcessDirectory')


def exists(env):
    return True
