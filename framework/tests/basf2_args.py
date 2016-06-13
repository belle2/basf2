#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import os

assert 0 == os.system('basf2 --info')

assert 0 == os.system('basf2 --version')

assert 0 == os.system('basf2 --help')

assert 0 == os.system('basf2 -m RootOutput')

testdir = Belle2.FileSystem.findFile('/framework/tests/')
os.chdir(testdir)
assert 0 == os.system('basf2 root_input.py --dry-run -i miep.root')
assert 0 == os.system('basf2 --dry-run root_input.py')
# TODO: check output from --dry-run to see if it changes


# some failing things
# exact return code of system() not defined, just unsuccesful
assert 0 != os.system('basf2 -m NonExistingModule')

assert 0 != os.system('basf2 --thisdoesntexist')

assert 0 != os.system('basf2 /this/path/doesnot/exist.py')
