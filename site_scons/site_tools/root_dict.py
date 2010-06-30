import os, re
from SCons.Builder import Builder

# regular expression to find class names in linkdef files
linkdef_class_re = re.compile(r'^#pragma\s+link\s+C\+\+\s+class\s+Belle2::(\S+);\s*$', re.M)

# define emitter for linkdef files:
# add all header files for classes which are listed in the linkdef file
def linkdef_emitter(target, source, env):
    sources = []
    contents = source[0].get_text_contents()
    source_dir = os.path.dirname(str(source[0]))
    for entry in linkdef_class_re.findall(contents):
        header_file = os.path.join(source_dir, entry.split('<')[0] + '.h')
        if not header_file in sources:
            sources.append(header_file)
    sources.append(source[0])
    return target, sources

# define builder for root dictionaries
rootcint = Builder(action = "rootcint -f $TARGET -c $_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES",
                   emitter = linkdef_emitter)

def generate(env):
    env['BUILDERS']['RootDict'] = rootcint

def exists(env):
    return True
