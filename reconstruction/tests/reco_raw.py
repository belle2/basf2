#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import glob
from ROOT import Belle2

if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
    print("TEST SKIPPED: BELLE2_VALIDATION_DATA_DIR environment variable not set.", file=sys.stderr)
    sys.exit(1)

steering = Belle2.FileSystem.findFile('reconstruction/tests/reco_raw.py_noexec')

for data_file in glob.glob(os.environ['BELLE2_VALIDATION_DATA_DIR'] + '/rawdata/*HLT?.*.root'):
    data_type = os.path.basename(data_file).split('.')[0]
    assert(0 == os.system("basf2 -n 10 -i " + data_file + ' ' + steering + ' ' + data_type))
