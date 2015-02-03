#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import tempfile
import shutil
import glob
from basf2 import *
from ROOT import Belle2

inputFile = Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root')
steeringFile = Belle2.FileSystem.findFile('analysis/tests/fei_mini_cache_steeringfile.internal')

tempdir = tempfile.mkdtemp()
print tempdir
os.chdir(tempdir)

shutil.copy(inputFile, "inputdata.root")

cmd = "basf2 " + steeringFile + " -i inputdata.root -o inputdata.root -- -cache cache.pkl --preload"

#fsp variablestontuple
assert 0 == os.system(cmd)
assert len(glob.glob('cache.pkl')) == 1
assert len(glob.glob('mcParticlesCount.root')) == 1
#fsp TMVATeacher
assert 0 == os.system(cmd)
#fsp training, final variablestontuple
assert 0 == os.system(cmd)
assert len(glob.glob('weights/*')) == 4
assert len(glob.glob('var*generic*.root*')) == 2
assert len(glob.glob('analysisPathDone.root')) == 1
os.unlink('analysisPathDone.root')

#create full path and run again
#mostly just need to omit --preload, usually one would use --dump-path to get a stable version
cmd = "basf2 " + steeringFile + " -i " + inputFile + " -- -cache cache.pkl"
assert 0 == os.system(cmd)
assert len(glob.glob('analysisPathDone.root')) == 1

shutil.rmtree(tempdir)
