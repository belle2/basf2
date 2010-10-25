import os
from SCons.Script import *


def process_dir(parent_env, dir_name):
    
    # determine library name
    lib_name = dir_name.replace(os.sep, '_')


    # get list of header and linkdef files
    header_files = Glob(os.path.join(dir_name, '*.h')) + Glob(os.path.join(dir_name, 'geometry', '*.h'))
    linkdef_files = []
    for header_file in header_files:
        if str(header_file).lower().endswith('linkdef.h'):
            linkdef_files.append(header_file)
            header_files.remove(header_file)

    # get list of source files
    src_nodes = Glob(os.path.join(dir_name, '*.cxx')) + Glob(os.path.join(dir_name, 'src', '*.cxx'))
    src_files = [os.path.join(parent_env['BUILDDIR'], str(node)) for node in src_nodes]


    # create environment for directory
    env = parent_env.Clone()
    env['HEADER_FILES'] = header_files
    env['LINKDEF_FILES'] = linkdef_files
    env['SRC_FILES'] = src_files


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
    inc_dir_name = ''
    if dir_name.find(os.sep) > 0:
        inc_dir_name = dir_name[dir_name.find(os.sep)+1:]
    includes = env.Install(os.path.join(env['INCDIR'], inc_dir_name), env['HEADER_FILES'])

    # loop over subdirs
    entries = os.listdir(dir_name)
    for entry in entries:
        if entry.find('.') == -1 and not os.path.isfile(os.path.join(dir_name, entry)) and not entry in ['include', 'src']:
            process_dir(env, os.path.join(dir_name, entry))

    # check whether we have to create a new library
    if (dir_name == env['PACKAGE']):
    
        # generate dictionaries
        dict_files = []
        for linkdef_file in env['LINKDEF_FILES']:
            dict_filename = str(linkdef_file).replace(os.sep, '_')[:-9] + 'Dict.cc'
            dict_file = env.RootDict(os.path.join(env['BUILDDIR'], dict_filename), linkdef_file)
            # add current directory to include path for dictionary compilation
            dict_files.append(env.SharedObject(dict_file, CPPPATH = ['.'] + env['CPPPATH']))

        # build a shared library with all source and dictionary files
        if len(env['SRC_FILES']) > 0:
            lib = env.SharedLibrary(os.path.join(env['LIBDIR'], lib_name), [env['SRC_FILES'], dict_files])


    # add linkdef, and source files to parent environment if we are in a normal sub-directory
    else:
        parent_env['LINKDEF_FILES'] += linkdef_files
        parent_env['SRC_FILES'] += src_files

def generate(env):
    env.AddMethod(process_dir, 'ProcessDirectory')

def exists(env):
    return True
