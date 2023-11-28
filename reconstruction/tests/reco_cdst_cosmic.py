#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test reconstruction of cosmic run.
'''

import basf2
import b2test_utils as b2u
import glob
import os
import subprocess as sp
import multiprocessing as mp
import random

if __name__ == '__main__':
    # Skip this test if needed.
    if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
        b2u.skip_test('BELLE2_VALIDATION_DATA_DIR environment variable not set, skipping the test.')
    # First, let's check in a smart way how many cores we can use.
    num_workers = max(int(mp.cpu_count() / (1. + os.getloadavg()[1])), 1)
    basf2.B2INFO(f'The test will be executed using {num_workers} workers.')
    # Then, let's run the test!
    steering_file = basf2.find_file('reconstruction/tests/reco_cdst.py_noexec')
    rawdata_files = glob.glob(os.environ['BELLE2_VALIDATION_DATA_DIR'] + '/rawdata/cosmic.*HLT?.*.root')
    rawdata_file = random.choice(rawdata_files)
    basf2.B2INFO(f'Running the test using {rawdata_file} as input file.')
    assert(sp.call(['basf2', steering_file, rawdata_file, 'cosmic']) == 0)
