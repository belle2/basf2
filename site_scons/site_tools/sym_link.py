#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from SCons.Script import GetOption

# define relpath for python < 2.6
if not hasattr(os.path, 'relpath'):


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

    os.symlink(os.path.relpath(source, os.path.dirname(target)), target)
    return 0


def generate(env):
    if not GetOption('no-symlink'):
        env['INSTALL'] = create_symlink


def exists(env):
    return True


