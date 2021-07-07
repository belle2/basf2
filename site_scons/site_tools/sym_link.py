#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import errno
from SCons.Script import GetOption
from SCons.Builder import Builder

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

    source = os.path.relpath(source, os.path.dirname(target))
    try:
        os.symlink(source, target)
    except OSError as e:
        # target might exist if source was moved!
        if e.errno == errno.EEXIST:
            os.remove(target)
            os.symlink(source, target)
        else:
            raise e
    return 0


copy_file = Builder(action='cp -pf $SOURCE $TARGET')
copy_file.action.cmdstr = '${COPYCOMSTR}'


def generate(env):
    if not GetOption('no-symlink'):
        env['INSTALL'] = create_symlink
    env['BUILDERS']['Copy'] = copy_file


def exists(env):
    return True
