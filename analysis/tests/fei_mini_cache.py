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

shutil.copy(inputFile, "inputdata.root")

cmd = "basf2 " + steeringFile + " -i inputdata.root -o inputdata.root -- -cache cache.pkl -verbose"

# fsp pre cut hist maker
assert 0 == os.system(cmd)
assert len(glob.glob('cache.pkl')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1
# fsp training data
assert 0 == os.system(cmd)
# fsp D pre cut hist maker and fsp training
assert 0 == os.system(cmd)
assert len(glob.glob('weights/*')) == 6
assert len(glob.glob('CutHistograms_D0*.root')) == 3
assert len(glob.glob('var_*')) == 3
# D channels training ignored and tag unique signal.
assert 0 == os.system(cmd)
assert len(glob.glob('D0*.root')) == 0
# Summary.
assert 0 == os.system(cmd)
assert len(glob.glob('Summary*')) == 1

# create full path and run again without cache
cmd = "basf2 " + steeringFile + " -i " + inputFile
assert 0 == os.system(cmd)
assert len(glob.glob('analysisPathDone.root')) == 1

shutil.rmtree(tempdir)
