#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from SCons.Builder import Builder

# define relpath for python < 2.6
if not hasattr(os, 'relpath'):


    def relpath(path, start=os.path.curdir):
        """Return a relative version of a path"""

        if not path:
            raise ValueError('no path specified')

        start_list = os.path.abspath(start).split(os.path.sep)
        path_list = os.path.abspath(path).split(os.path.sep)

        # Work out how much of the filepath is shared by start and path.
        i = len(os.path.commonprefix([start_list, path_list]))

        rel_list = [os.path.pardir] * (len(start_list) - i) + path_list[i:]
        if not rel_list:
            return os.path.curdir
        return os.path.join(*rel_list)


os.path.relpath = relpath


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


