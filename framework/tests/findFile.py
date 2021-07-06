#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import find_file, B2INFO
import os

# define shortcut for getting symlink-free path
rl = os.path.realpath


def expect_missing(*args):
    """Return true if passing the arguments to find_file raises a FileNotFoundError, otherwise False"""
    # raises a file not found exception if not found (unless supressed)
    try:
        find_file(*args)
    except FileNotFoundError:
        return True
    else:
        return False


assert expect_missing('framework/tests/ThisFileDoesntExist')
# try to find TEST data file
assert expect_missing('findFile.py', 'test')

B2INFO("No more output from here...")
assert '' == find_file('framework/tests/ThisFileDoesntExist', silent=True)

# find ourselves relative to basf2 dir
abspath = rl(find_file('framework/tests/findFile.py'))
assert len(abspath) > 0

# adding '/' is ok, but optional
assert abspath == rl(find_file('/framework/tests/findFile.py'))

# absolute paths should be found
assert abspath == rl(find_file(abspath))

# also works on directories
testdir = rl(find_file('/framework/tests/'))
assert len(testdir) > 0

# paths relative to PWD are also ok (no, prefixing this with / is not allowed)
os.chdir(testdir)
assert abspath == rl(find_file('findFile.py'))
assert abspath == rl(find_file('./findFile.py'))

# try finding relative to basf2 dir again (from different location)
os.chdir('/')
assert abspath == rl(find_file('framework/tests/findFile.py'))
assert abspath == rl(find_file('/framework/tests/findFile.py'))
assert abspath == rl(find_file('./framework/tests/findFile.py'))

# set BELLe2_TEST_DATA_DIR and try to find data file
os.environ['BELLE2_TEST_DATA_DIR'] = os.environ.get('BELLE2_RELEASE_DIR', os.environ.get('BELLE2_LOCAL_DIR'))
assert abspath == rl(find_file('framework/tests/findFile.py', 'test'))
