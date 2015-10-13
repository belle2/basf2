#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import subprocess
import tempfile
import shutil
import glob
from basf2 import *
from ROOT import Belle2

inputFile = Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root')
steeringFile = Belle2.FileSystem.findFile('analysis/tests/fei_mini_steeringfile.internal')

tempdir = tempfile.mkdtemp()
print(tempdir)
os.chdir(tempdir)

cmd = "basf2 " + steeringFile + " -i " + inputFile + " -- -verbose"

# fsp variablestontuple
assert 0 == os.system(cmd)
# fsp TMVATeacher
assert 0 == os.system(cmd)
# fsp training and D hists
assert 0 == os.system(cmd)
assert len(glob.glob('weights/*')) == 6
assert len(glob.glob('CutHistograms_D0*.root')) == 2
# D channels ignored.
assert 0 == os.system(cmd)
assert len(glob.glob('D0*.root')) == 0

shutil.rmtree(tempdir)
