#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

# Test redirect_stdout, as we use that in check_basf2_file
from contextlib import redirect_stdout
with redirect_stdout(open(os.devnull, 'w')):
    print('abc')

from basf2 import *
from ROOT import Belle2


rootfile = Belle2.FileSystem.findFile('/framework/tests/root_input.root')
assert 0 == os.system('check_basf2_file ' + rootfile)

assert 0 != os.system('check_basf2_file /THIS_DOESNT_EXIST')
assert 0 != os.system('check_basf2_file /dev/null')


assert 0 == os.system('check_basf2_file -n 5 ' + rootfile)
assert 0 == os.system('check_basf2_file --events 5 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 10 ' + rootfile)

# sadly, our input file doesn't have mcEvents stored (==0)
assert 0 == os.system('check_basf2_file --mcevents 0 ' + rootfile)
assert 0 != os.system('check_basf2_file --mcevents 123 ' + rootfile)

assert 0 == os.system('check_basf2_file -n 5 -s 12.4 1 ' + rootfile)
assert 0 == os.system('check_basf2_file -n 5 -s 12.4 0.05 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 5 -s 12.4 0 ' + rootfile)

assert 0 == os.system('check_basf2_file -n 5 -s 10 1 ' + rootfile)
assert 0 == os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 5 -s 10 0.2 ' + rootfile)
assert 0 != os.system('check_basf2_file -n 5 -s 10 0.05 ' + rootfile)

assert 0 == os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile + ' EventMetaData')
allbranches = 'EventMetaData PXDClusters PXDClustersToPXDDigits PXDClustersToPXDTrueHits PXDDigits PXDTrueHits'
assert 0 == os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile + ' ' + allbranches)
assert 0 == os.system('check_basf2_file --json -n 5 -s 10 0.5 ' + rootfile + ' ' + allbranches)

assert 0 != os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile + ' NotThere')
assert 0 != os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile + ' EventMetaData NotThere')
assert 0 != os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile + ' NotThere EventMetaData')
assert 0 != os.system('check_basf2_file -n 5 -s 10 0.5 ' + rootfile + ' FileMetaData')
