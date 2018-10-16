#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
from SCons.Builder import Builder
from SCons.Scanner.C import CScanner
from SCons.Action import CommandGeneratorAction

# everything that has #pragma link C++ .* Belle2::.*
linkdef_everything = \
    re.compile(r'^#pragma\s+link\s+C\+\+\s+[\w]*\s+Belle2::.*$', re.M)

# regular expression to find class names in linkdef files
linkdef_class_re = \
    re.compile(r'^#pragma\s+link\s+C\+\+\s+[\w]*\s+Belle2::([\w]*::)?([\w]*).*[+-]?\!?;.*$', re.M)

# link requests with '// implicit' comment (doesn't require header file)
linkdef_implicit = \
    re.compile(r'^#pragma\s+link\s+C\+\+\s+[\w]*\s+Belle2::.*;\s*//\s*implicit\s*$', re.M)

# custom generator for the Root dictionary builder
# it will also generate the .rootmap files to allow ROOT
# to auto-load libraries


def rootcling_generator(source, target, env, for_signature):
    # extract the name of the root library which is generated at this stage
    # this variable will be set by the process_dir function
    rootmap_libname = env["ROOTCLING_ROOTMAP_LIB"]
    # use the .cc-dict file name (in target[0]) to generate the .rootmap file
    cmd = 'rootcling -f $TARGET $CLINGFLAGS -rmf "{0}.rootmap" -rml lib{1}.so $_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES' \
        .format(str(target[0]).replace(".cc", ""), rootmap_libname)
    return cmd


def linkdef_emitter(target, source, env):
    # determine the right include directory
    source_dir = os.path.dirname(str(source[0]))
    include_dir = source_dir
    if include_dir.endswith('include'):
        include_dir = os.path.dirname(include_dir)
    include_dir = os.path.join(env['INCDIR'], include_dir)

    # loop over class names and construct the corresponding header file names
    sources = []
    contents = source[0].get_text_contents()
    for line in contents.split('\n'):
        # first check if this is looks like an actual request to create a dictionary
        if linkdef_everything.search(line) is None:
            continue

        match = linkdef_class_re.search(line)
        if match is None:
            raise RuntimeError(
                "%s contains '%s' which we couldn't parse. The syntax may be incorrect," +
                " or the build system may lack support for the feature you're using." %
                (str(
                    source[0]),
                    line))

        namespace = match.group(1)  # or possibly a class, but it might match the header file
        classname = match.group(2)
        if not classname:
            raise RuntimeError("%s contains '%s' without class name?"
                               % (str(source[0]), str(line)))

        is_implicit = not linkdef_implicit.search(line) is None
        if is_implicit:
            continue  # do not look for a header for this request

        include_base = classname + '.h'
        header_file = os.path.join(source_dir, include_base)
        if not os.path.isfile(header_file):
            header_file = os.path.join(os.environ.get('BELLE2_RELEASE_DIR', ''
                                                      ), header_file)
        if not os.path.isfile(header_file):
            if not namespace:
                # no //implicit for suppression found...
                print("%s contains '%s' where we couldn't find a header file. "
                      "If dictionary compilation fails, this might be the reason. "
                      "For classes residing in other directories and already "
                      "included via other link requests, add '// implicit' at "
                      "the end to suppress this message." % (str(source[0]), str(line)))
                continue

            # remove trailing '::', add '.h'
            include_base = namespace.split(':')[0] + '.h'
        include_file = os.path.join(include_dir, include_base)
        if include_file not in sources:
            sources.append(include_file)

    sources.append(source[0])
    return (target, sources)


# define builder for root dictionaries
rootcint = Builder(action=CommandGeneratorAction(rootcling_generator, {'cmdstr': '${ROOTCINTCOMSTR}'}),
                   emitter=linkdef_emitter, source_scanner=CScanner())


def generate(env):
    env['BUILDERS']['RootDict'] = rootcint


def exists(env):
    return True
