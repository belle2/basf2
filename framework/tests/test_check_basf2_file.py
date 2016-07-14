#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import os

rootfile = Belle2.FileSystem.findFile('/framework/tests/root_input.root')
assert 0 == os.system('check_basf2_file ' + rootfile)

assert 0 != os.system('check_basf2_file /THIS_DOESNT_EXIST')
assert 0 != os.system('check_basf2_file /dev/null')


assert 0 == os.system('check_basf2_file -n 5 ' + rootfile)
assert 0 == os.system('check_basf2_file --events 5 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 10 ' + rootfile)

assert 0 == os.system('check_basf2_file -n 5 -s 12.4 1 ' + rootfile)
assert 0 == os.system('check_basf2_file -n 5 -s 12.4 0.05 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 5 -s 12.4 0 ' + rootfile)

assert 0 == os.system('check_basf2_file -n 5 -s 10 1 ' + rootfile)
assert 0 == os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 5 -s 10 0.2 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 5 -s 10 0.05 ' + rootfile)
