#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os

# Test redirect_stdout, as we use that in b2file-check
from contextlib import redirect_stdout
with redirect_stdout(open(os.devnull, 'w')):
    print('abc')

import basf2  # noqa


rootfile = basf2.find_file('/framework/tests/root_input.root')
assert 0 == os.system('b2file-check ' + rootfile)

assert 0 != os.system('b2file-check /THIS_DOESNT_EXIST')
assert 0 != os.system('b2file-check /dev/null')


assert 0 == os.system('b2file-check -n 5 ' + rootfile)
assert 0 == os.system('b2file-check --events 5 ' + rootfile)
assert 0 != os.system('b2file-check -n 10 ' + rootfile)

# sadly, our input file doesn't have mcEvents stored (==0)
assert 0 == os.system('b2file-check --mcevents 0 ' + rootfile)
assert 0 != os.system('b2file-check --mcevents 123 ' + rootfile)

assert 0 == os.system('b2file-check -n 5 -s 12.4 1 ' + rootfile)
assert 0 == os.system('b2file-check -n 5 -s 12.4 0.05 ' + rootfile)
assert 0 != os.system('b2file-check -n 5 -s 12.4 0 ' + rootfile)

assert 0 == os.system('b2file-check -n 5 -s 10 1 ' + rootfile)
assert 0 == os.system('b2file-check -n 5 -s 10 0.5 ' + rootfile)
assert 0 != os.system('b2file-check -n 5 -s 10 0.2 ' + rootfile)
assert 0 != os.system('b2file-check -n 5 -s 10 0.05 ' + rootfile)

assert 0 == os.system('b2file-check -n 5 -s 10 0.5 ' + rootfile + ' EventMetaData')
allbranches = 'EventMetaData PXDClusters PXDClustersToPXDDigits PXDClustersToPXDTrueHits PXDDigits PXDTrueHits'
assert 0 == os.system('b2file-check -n 5 -s 10 0.5 ' + rootfile + ' ' + allbranches)
assert 0 == os.system('b2file-check --json -n 5 -s 10 0.5 ' + rootfile + ' ' + allbranches)

assert 0 != os.system('b2file-check -n 5 -s 10 0.5 ' + rootfile + ' NotThere')
assert 0 != os.system('b2file-check -n 5 -s 10 0.5 ' + rootfile + ' EventMetaData NotThere')
assert 0 != os.system('b2file-check -n 5 -s 10 0.5 ' + rootfile + ' NotThere EventMetaData')
assert 0 != os.system('b2file-check -n 5 -s 10 0.5 ' + rootfile + ' FileMetaData')
