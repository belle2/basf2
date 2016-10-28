#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import os

# define shortcut for getting symlink-free path
rl = os.path.realpath

# returns empty string if not found
assert '' == Belle2.FileSystem.findFile('framework/tests/ThisFileDoesntExist')

# find ourselves relative to basf2 dir
abspath = rl(Belle2.FileSystem.findFile('framework/tests/findFile.py'))
assert len(abspath) > 0

# adding '/' is ok, but optional
assert abspath == rl(Belle2.FileSystem.findFile('/framework/tests/findFile.py'))

# absolute paths should be found
assert abspath == rl(Belle2.FileSystem.findFile(abspath))

# also works on directories
testdir = rl(Belle2.FileSystem.findFile('/framework/tests/'))
assert len(testdir) > 0

# paths relative to PWD are also ok (no, prefixing this with / is not allowed)
os.chdir(testdir)
print(
    'PWD: ',
    testdir,
    'findfile exists: ',
    os.path.isfile(
        testdir +
        '/findFile.py'),
    'returned path: ',
    Belle2.FileSystem.findFile('findFile.py'))
assert abspath == rl(Belle2.FileSystem.findFile('findFile.py'))

# try finding relative to basf2 dir again (from different location)
os.chdir('/')
assert abspath == rl(Belle2.FileSystem.findFile('framework/tests/findFile.py'))
assert abspath == rl(Belle2.FileSystem.findFile('/framework/tests/findFile.py'))
