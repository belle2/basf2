#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
from SCons.Builder import Builder
from SCons.Scanner.C import CScanner

# regular expression to find class names in linkdef files
linkdef_class_re = \
    re.compile('^#pragma\\s+link\\s+C\\+\\+\\s+class\\s+Belle2::([\\w]*::)?([\\w<>,\\*]+)[+-]?\\!?;.*$'
               , re.M)


def linkdef_emitter(target, source, env):
    # emitter for linkdef files:
    # add all header files for classes which are listed in the linkdef file

    # determine the right include directory
    source_dir = os.path.dirname(str(source[0]))
    include_dir = source_dir
    if include_dir.endswith('include'):
        include_dir = os.path.dirname(include_dir)
    include_dir = os.path.join(env['INCDIR'], include_dir)

    # loop over class names and construct the corresponding header file names
    sources = []
    contents = source[0].get_text_contents()
    for entry in linkdef_class_re.findall(contents):
        include_base = entry[1].split('<')[0] + '.h'
        header_file = os.path.join(source_dir, include_base)
        if not os.path.isfile(header_file):
            header_file = os.path.join(os.environ.get('BELLE2_RELEASE_DIR', ''
                                       ), header_file)
        if not os.path.isfile(header_file):
            include_base = entry[0].split(':')[0] + '.h'
        include_file = os.path.join(include_dir, include_base)
        if not include_file in sources:
            sources.append(include_file)

    sources.append(source[0])
    return (target, sources)


# define builder for root dictionaries
rootcint = \
    Builder(action='rootcint -f $TARGET -c -p $CINTFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS $SOURCES'
            , emitter=linkdef_emitter, source_scanner=CScanner())
rootcint.action.cmdstr = '${ROOTCINTCOMSTR}'


def generate(env):
    env['BUILDERS']['RootDict'] = rootcint


def exists(env):
    return True


