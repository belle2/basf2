#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import glob
import subprocess
import basf2 as b2
import b2test_utils as b2u
import ROOT.Belle2 as B2

if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
    b2u.skip_test("BELLE2_VALIDATION_DATA_DIR environment variable not set.")

steering_file = B2.FileSystem.findFile('reconstruction/tests/reco_cdst.py_noexec')

for rawdata_file in glob.glob(os.environ['BELLE2_VALIDATION_DATA_DIR'] + '/rawdata/*HLT?.*.root'):
    b2.B2INFO(f'Running the test using {rawdata_file} as input file.')
    if 'cosmic' in os.path.basename(rawdata_file):
        assert(0 == subprocess.check_call(['basf2', steering_file, rawdata_file, 'cosmic']))
    elif 'physics' in os.path.basename(rawdata_file):
        assert(0 == subprocess.check_call(['basf2', steering_file, rawdata_file, 'physics']))
    else:
        b2.B2INFO('Not a "cosmic" neither a "physics" run, skipping it.')
