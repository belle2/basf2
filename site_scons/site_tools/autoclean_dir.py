#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from SCons import Node


def autoclean_dir(env, dirname):
    """
    Autoclean directory, deleting all files which are not known to the build
    system, like obsolete header files
    """

    # First make a set of targets which are found inside the directory and all its subdirs
    directory = env.Dir(dirname)
    targets = set()
    stack = [directory]
    while stack:
        dirnode = stack.pop()
        for node in dirnode.all_children():
            if isinstance(node, Node.FS.Dir):
                stack.append(node)
            elif node.has_explicit_builder():
                targets.add(str(node))

    # Now delete all files found in the directory which are not in the target set
    for (dirname, dirs, files) in os.walk(str(directory)):
        if "__pycache__" in dirs:
            dirs.remove("__pycache__")
        for filename in files:
            fullname = os.path.join(dirname, filename)
            if fullname not in targets and not filename.startswith('.') and not \
                    (fullname.endswith('.pyc') and fullname[:-1] in targets):
                print env['CLEANUPCOMSTR'] % fullname
                os.remove(fullname)


def generate(env):
    env.AddMethod(autoclean_dir, 'RemoveNonTargets')


def exists(env):
    return True
