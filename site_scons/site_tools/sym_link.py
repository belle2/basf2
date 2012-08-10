#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from SCons.Builder import Builder


def create_symlink(target, source, env):
    """Create a symbolic link from source to target"""

    os.symlink(os.path.relpath(str(source[0]),
               os.path.dirname(str(target[0]))), str(target[0]))
    return None


# define builder for symbolic links
symlink_builder = Builder(action=create_symlink)
symlink_builder.action.cmdstr = '${SYMLINKCOMSTR}'


def generate(env):
    env['BUILDERS']['SymLink'] = symlink_builder


def exists(env):
    return True


