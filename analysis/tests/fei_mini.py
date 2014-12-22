#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import tempfile
import shutil
import glob
from basf2 import *
from ROOT import Belle2

inputFile = Belle2.FileSystem.findFile('analysis/tests/mdst_100events.root')
steeringFile = Belle2.FileSystem.findFile('analysis/tests/fei_mini_steeringfile.internal')

tempdir = tempfile.mkdtemp()
print tempdir
os.chdir(tempdir)

cmd = "basf2 " + steeringFile + " -i " + inputFile + " -- -summary"

#fsp variablestontuple
assert 0 == os.system(cmd)
#fsp TMVATeacher
assert 0 == os.system(cmd)
#fsp training and D hists
assert 0 == os.system(cmd)
assert len(glob.glob('weights/*')) == 4
assert len(glob.glob('CutHistograms_D0*.root')) == 1
#D channels ignored.
assert 0 == os.system(cmd)
assert len(glob.glob('D0*.root')) == 0

shutil.rmtree(tempdir)
