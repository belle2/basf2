import os, re
from SCons.Builder import Builder

# regular expression to find module names in source files
module_name_re = re.compile(r'REG_MODULE\((\S+)\)', re.M)

# extract the module names from all source files
def build_map(target, source, env):

    # create target output file
    map_file = open(str(target[0]), 'w')

    # loop over source files and extract registered modules
    for source_file in source:
        contents = source_file.get_text_contents()
        for entry in module_name_re.findall(contents):
            map_file.write(entry + '\n')

    map_file.close()

    return None

# define builder for module map files
map_builder = Builder(action = build_map)

def generate(env):
    env['BUILDERS']['ModuleMap'] = map_builder

def exists(env):
    return True
