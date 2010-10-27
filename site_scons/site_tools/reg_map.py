#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import re
from SCons.Builder import Builder

# regular expression to find module names in source files
reg_re = re.compile(r'(REG_[A-Z0-9_]+\([\S ]+\))', re.M)


def build_map(target, source, env):
    # extract the module names from all source files

    # create target output file
    map_file = open(str(target[0]), 'w')

    # loop over source files and extract registered modules
    for source_file in source:
        contents = source_file.get_text_contents()
        for entry in reg_re.findall(contents):
            map_file.write(entry + '\n')

    map_file.close()

    return None


# define builder for module map files
map_builder = Builder(action=build_map)
map_builder.action.cmdstr = '${MAPCOMSTR}'


def generate(env):
    env['BUILDERS']['RegMap'] = map_builder


def exists(env):
    return True


