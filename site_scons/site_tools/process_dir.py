import os
from SCons.Script import *


def define_aliases(env, target, dir_name, extension = None):
    parent_dir = dir_name
    while len(parent_dir) > 0:
        env.Alias(parent_dir, target)
        if extension:
            env.Alias(parent_dir + extension, target)
        parent_dir = os.path.split(parent_dir)[0]

def process_dir(parent_env, dir_name):
    
    # determine library name
    lib_name = dir_name.replace(os.sep, '_')


    # get list of header and linkdef files
    header_files = Glob(os.path.join(dir_name, '*.h')) + Glob(os.path.join(dir_name, 'include', '*.h'))
    linkdef_files = []
    for header_file in header_files:
        if str(header_file).lower().endswith('linkdef.h'):
            linkdef_files.append(header_file)
            header_files.remove(header_file)

    # get list of source files
    src_nodes = Glob(os.path.join(dir_name, '*.cc')) + Glob(os.path.join(dir_name, 'src', '*.cc'))
    src_files = [os.path.join(parent_env['BUILDDIR'], str(node)) for node in src_nodes]

    # get list of script files
    script_files = Glob(os.path.join(dir_name, 'scripts', '*'))

    # get list of data files
    data_files = Glob(os.path.join(dir_name, 'data', '*'))


    # create environment for directory
    env = parent_env.Clone()
    env['HEADER_FILES'] = header_files
    env['LINKDEF_FILES'] = linkdef_files
    env['SRC_FILES'] = src_files
    env['SCRIPT_FILES'] = script_files
    env['DATA_FILES'] = data_files

    # include SConscript file if it exists
    if os.path.isfile(os.path.join(dir_name, 'SConscript')):
        result = SConscript(os.path.join(dir_name, 'SConscript'), exports = 'env')

        # use the new environment if it was updated by the SConscript file
        if isinstance(result, Environment):
            env = result

            # don't continue with the default build process if the SConscript file requests this
            if env.Dictionary().has_key('CONTINUE') and env['CONTINUE'] == False:
                return


    # install header files in the include directory
    includes = env.Install(os.path.join(env['INCDIR'], dir_name), env['HEADER_FILES'])
    define_aliases(env, includes, dir_name, '.include')

    # install script files in the library directory
    data = env.Install(env['LIBDIR'], env['SCRIPT_FILES'])
    define_aliases(env, data, dir_name, '.scripts')

    # install data files in the data directory
    data = env.Install(os.path.join(env['DATADIR'], dir_name), env['DATA_FILES'])
    define_aliases(env, data, dir_name, '.data')

    # loop over subdirs
    entries = os.listdir(dir_name)
    for entry in entries:
        if entry.find('.') == -1 and not os.path.isfile(os.path.join(dir_name, entry)) and not entry in ['include', 'src', 'tools', 'data']:
            process_dir(env, os.path.join(dir_name, entry))

    # check whether we have to create a new library
    if (dir_name == env['PACKAGE']) or (env.Dictionary().has_key('SUBLIB') and (env['SUBLIB'] == True) or dir_name.endswith('modules')):
    
        # generate dictionaries
        dict_files = []
        for linkdef_file in env['LINKDEF_FILES']:
            dict_filename = str(linkdef_file)[:-9] + 'Dict.cc'        
            dict_file = env.RootDict(os.path.join(env['BUILDDIR'], dict_filename), linkdef_file)
            # add current directory to include path for dictionary compilation
            dict_files.append(env.SharedObject(dict_file, CPPPATH = ['.'] + env['CPPPATH']))

        # build a shared library with all source and dictionary files
        if len(env['SRC_FILES']) > 0:
            lib = env.SharedLibrary(os.path.join(env['LIBDIR'], lib_name), [env['SRC_FILES'], dict_files])
            env.Alias(lib_name, lib)
            define_aliases(env, lib, dir_name, '.lib')

    # add linkdef, and source files to parent environment if we are in a normal sub-directory
    else:
        parent_env['LINKDEF_FILES'] += linkdef_files
        parent_env['SRC_FILES'] += src_files


    # setup environment for building executables, include SConscript if it exists
    save_env = env.Clone()
    env['TOOLS_FILES'] = Glob(os.path.join(dir_name, 'tools', '*.cc'))
    if os.path.isfile(os.path.join(dir_name, 'tools', 'SConscript')):
        result = SConscript(os.path.join(dir_name, 'tools', 'SConscript'), exports = 'env')
        if isinstance(result, Environment):
            env = result

    # build a binary for each source file in the tools directory
    for bin_file in env['TOOLS_FILES']:
        bin_filename = os.path.splitext(os.path.basename(str(bin_file)))[0]
        bin_env = env.Clone()
        if bin_env['TOOLS_LIBS'].has_key(bin_filename):
            bin_env['LIBS'] = Flatten([env.subst(str(lib)).split() for lib in Flatten(bin_env['TOOLS_LIBS'][bin_filename])])
        if bin_env['TOOLS_LIBPATH'].has_key(bin_filename):
            bin_env['LIBPATH'] = bin_env['TOOLS_LIBPATH'][bin_filename]
        tool = bin_env.Program(os.path.join(bin_env['BINDIR'], bin_filename),
                               os.path.join(bin_env['BUILDDIR'], str(bin_file)))
        env.Alias(os.path.join(dir_name, 'tools', bin_filename), tool)
        env.Alias(os.path.join(dir_name, 'tools'), tool)
        env.Alias(os.path.join(dir_name, bin_filename), tool)
        define_aliases(env, tool, dir_name, '.bin')

    # restore original environment
    env = save_env

def generate(env):
    env.AddMethod(process_dir, 'ProcessDirectory')

def exists(env):
    return True
