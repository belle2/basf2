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
steeringFile = Belle2.FileSystem.findFile('analysis/tests/fei_mini_specific_steeringfile.internal')

tempdir = tempfile.mkdtemp()
print(tempdir)
os.chdir(tempdir)

cmd = "basf2 -p 2 " + steeringFile + " -i " + inputFile + " -- -summary -verbose"

# fsp variablestontuple
assert 0 == os.system(cmd)
assert len(glob.glob('mcParticlesCount.root')) == 1
# fsp TMVATeacher
assert 0 == os.system(cmd)
# fsp training, final variablestontuple
assert 0 == os.system(cmd)
assert 0 == os.system(cmd)
# fsp training, final variablestontuple
assert 0 == os.system(cmd)
assert len(glob.glob('weights/*')) == 4
assert len(glob.glob('var*.root*')) == 2
assert len(glob.glob('FEIsummary.tex')) == 1
os.unlink('FEIsummary.tex')

# create full path and run again
# mostly just need, usually one would use --dump-path to get a stable version
cmd = "basf2 -p 2 " + steeringFile + " -i " + inputFile
assert 0 == os.system(cmd)
assert len(glob.glob('analysisPathDone.root')) == 1

shutil.rmtree(tempdir)
