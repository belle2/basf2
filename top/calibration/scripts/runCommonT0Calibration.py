#!/usr/bin/env python
# -*- coding: utf-8 -*-

# -------------
# Basic script to submit the jobs for the CommonT0 offline calibration
# For expert useage only!
# Contributor: Umberto Tamponi
#

from basf2 import *
from ROOT import Belle2
from ROOT import TH1F, TProfile, TFile
from ROOT import gROOT, AddressOf
import math
import ROOT
import sys
import os


for iRun in range(123, 1200):
    folderName = '/hsm/belle2/bdata/Data/release-01-02-02/DBxxxxxxxx/prod00000001/e0003/4S/r' + \
        format(iRun, '05d') + '/all/cdst/sub00/'
    if os.path.isdir(folderName):
        fileName = ' -i ' + folderName + '*.root'
        print(fileName)
        os.system('bsub -q l basf2 calculateCommonCalibration.py ' + fileName)
