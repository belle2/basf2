#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import re
from SCons.Builder import Builder

# regular expression to find module names in source files
# corresponding code to get module name in ModuleManager::fillModuleNameLibMap()
reg_re = re.compile(r'^\s*(REG_[A-Z0-9_]+\s*\([\S ]+\))', re.M)


def build_map(target, source, env):
    # extract the module names from all source files

    # create target output file
    map_file = open(str(target[0]), 'w')

    # loop over source files and extract registered modules
    nModules = 0
    for source_file in source:
        # ignore libs
        if str(source_file).endswith(env.subst('$SHLIBSUFFIX')):
            continue

        contents = source_file.get_text_contents()
        for entry in reg_re.findall(contents):
            map_file.write(entry + '\n')
            nModules += 1

    map_file.close()

    if nModules == 0:
        return "Module map file empty. Did you forget REG_MODULE(...) in your module source file?"

    return None


# define builder for module map files
map_builder = Builder(action=build_map)
map_builder.action.cmdstr = '${MAPCOMSTR}'


def generate(env):
    env['BUILDERS']['RegMap'] = map_builder


def exists(env):
    return True
