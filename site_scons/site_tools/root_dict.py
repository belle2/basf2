#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Custom builder for the Root dictionaries.
# It will also generate the .rootmap files to allow ROOT
# to auto-load libraries

import os
import re
from SCons.Builder import Builder
from SCons.Scanner.C import CScanner

# Classes outside of namespace Belle2.
# Such classes can be linked using comment "// global".
linkdef_global = \
    re.compile(r'^#pragma\s+link\s+C\+\+\s+[\w]*\s+([\w]*).*[+-]?\!?;\s*//.*global.*$', re.M)

# everything that has #pragma link C++ .* Belle2::.*
linkdef_everything = \
    re.compile(r'^#pragma\s+link\s+C\+\+\s+[\w]*\s+Belle2::.*$', re.M)

# regular expression to find class names in linkdef files
linkdef_class_re = \
    re.compile(r'^#pragma\s+link\s+C\+\+\s+[\w]*\s+Belle2::([\w]*::)?([\w]*).*[+-]?\!?;.*$', re.M)

# link requests with '// implicit' comment (doesn't require header file)
linkdef_implicit = \
    re.compile(r'^#pragma\s+link\s+C\+\+\s+[\w]*\s+Belle2::.*;\s*//.*implicit.*$', re.M)


def linkdef_emitter(target, source, env):
    linkdef = source.pop()
    # determine the right include directory
    source_dir = os.path.dirname(str(linkdef))
    include_dir = source_dir
    if include_dir.endswith('include'):
        include_dir = os.path.dirname(include_dir)
    include_dir = os.path.join(env['INCDIR'], include_dir)

    dict_basename = os.path.splitext(str(target[0]))[0]
    # add rootmap file as extra target
    target.append(dict_basename + '.rootmap')
    # add root pcm file as extra target
    target.append(dict_basename + '_rdict.pcm')

    # loop over class names and construct the corresponding header file names
    contents = linkdef.get_text_contents()
    for line in contents.split('\n'):
        # Classes outside of namespace Belle2.
        match = linkdef_global.match(line)
        if match is not None:
            classname = match.group(1)
            if classname is not None:
                include_base = classname + '.h'
                header_file = os.path.join(source_dir, include_base)
                if os.path.isfile(header_file):
                    include_file = os.path.join(include_dir, include_base)
                    if include_file not in source:
                        source.append(include_file)
                else:
                    print(f'Cannot find header file for the line "{line}".')

        # first check if this is looks like an actual request to create a dictionary
        if linkdef_everything.search(line) is None:
            continue

        match = linkdef_class_re.search(line)
        if match is None:
            raise RuntimeError(
                f"{linkdef} contains '{line}' which we couldn't parse. The syntax may be incorrect,"
                " or the build system may lack support for the feature you're using.")

        namespace = match.group(1)  # or possibly a class, but it might match the header file
        classname = match.group(2)
        if not classname:
            raise RuntimeError("%s contains '%s' without class name?" % (str(linkdef), str(line)))

        is_implicit = not linkdef_implicit.search(line) is None
        if is_implicit:
            continue  # do not look for a header for this request

        include_base = classname + '.h'
        header_file = os.path.join(source_dir, include_base)

        if not os.path.isfile(header_file):
            if not namespace:
                # no //implicit for suppression found...
                print("%s contains '%s' where we couldn't find a header file. "
                      "If dictionary compilation fails, this might be the reason. "
                      "For classes residing in other directories and already "
                      "included via other link requests, add '// implicit' at "
                      "the end to suppress this message." % (str(linkdef), str(line)))
                continue

            # remove trailing '::', add '.h'
            include_base = namespace.split(':')[0] + '.h'
        include_file = os.path.join(include_dir, include_base)
        if include_file not in source:
            source.append(include_file)

    # make sure linkdef is last source because that's how rootcling wants it
    source.append(linkdef)
    return (target, source)


# define builder for root dictionaries
rootcling = Builder(
    action='rootcling --failOnWarnings -f $TARGET $CLINGFLAGS -rmf "${TARGET.base}.rootmap" -rml lib${ROOTCLING_ROOTMAP_LIB}.so '
    '$_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES',
    emitter=linkdef_emitter,
    source_scanner=CScanner())
rootcling.action.cmdstr = '${ROOTCLINGCOMSTR}'

# define builder for class version check
classversion = Builder(action='b2code-classversion-check --error-style gcc $SOURCE && touch $TARGET')
classversion.action.cmdstr = '${CLASSVERSIONCOMSTR}'


def generate(env):
    env['BUILDERS']['RootDict'] = rootcling
    env['BUILDERS']['ClassVersionCheck'] = classversion


def exists(env):
    return True
