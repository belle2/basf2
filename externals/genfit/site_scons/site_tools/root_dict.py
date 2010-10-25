import os, re
from SCons.Builder import Builder
from SCons.Scanner.C import CScanner

# regular expression to find class names in linkdef files
linkdef_class_re = re.compile(r'^#pragma\s+link\s+C\+\+\s+class\s+([\w<>,\*]+)[+-]?\!?;\s*$', re.M)

# define emitter for linkdef files:
# add all header files for classes which are listed in the linkdef file
def linkdef_emitter(target, source, env):

    # loop over class names and construct the corresponding header file names
    sources = []
    contents = source[0].get_text_contents()
    for entry in linkdef_class_re.findall(contents):
        include_base = entry.split('<')[0] + '.h'
        include_file = os.path.join(env['INCDIR'], include_base)
        if not include_file in sources:
            sources.append(include_file)

    sources.append(source[0])
    return target, sources

# define builder for root dictionaries
rootcint = Builder(action = "rootcint -f $TARGET -c $_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES",
                   emitter = linkdef_emitter,
                   source_scanner = CScanner())

def generate(env):
    env['BUILDERS']['RootDict'] = rootcint

def exists(env):
    return True
