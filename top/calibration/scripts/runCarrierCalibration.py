#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------
#
# Master script to run the carrier Calibration over a gorup of iovs
#
# Contributor: Umberto Tamponi (tamponi@to.infn.it)
#
# --------------------

from basf2 import *
from ROOT import Belle2
from ROOT import TH1F, TProfile, TFile
from ROOT import gROOT, AddressOf
import math
import ROOT
import sys
import os

# Iovs to be analyzed
runRanges = [range(123, 140 + 1),
             range(314, 314 + 1),
             range(479, 492 + 1),
             range(521, 532 + 1),
             range(573, 580 + 1),
             range(666, 686 + 1),
             range(781, 786 + 1),
             range(969, 972 + 1),
             range(1158, 1162 + 1)
             ]

# jobs submission
for runRange in runRanges:
    fileName = ''
    for iRun in runRange:
        folderName = '/hsm/belle2/bdata/Data/release-01-02-02/DBxxxxxxxx/prod00000001/e0003/4S/r' + \
            format(iRun, '05d') + '/all/cdst/sub00/'
        if os.path.isdir(folderName):
            fileName = fileName + ' -i ' + folderName + '*.root'
    print(fileName)
    os.system('bsub -q l basf2 calibrateCarriers.py  ' + fileName)
