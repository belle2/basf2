#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__author__ = "S. Jia"

import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *
from stdCharged import stdPi, stdK, stdE, stdMu

# create a new path
ISRskimpath = Path()

fileList = ['../ISRpipimumu.dst.root']

inputMdstList('MC9', fileList, path=ISRskimpath)

# use standard final state particle lists
stdPi('loose', path=ISRskimpath)
stdK('loose', path=ISRskimpath)
stdE('loose', path=ISRskimpath)
stdMu('loose', path=ISRskimpath)
stdPi('all', path=ISRskimpath)
stdK('all', path=ISRskimpath)
stdE('all', path=ISRskimpath)
stdMu('all', path=ISRskimpath)

# importing the reconstructed events from the ISRpipicc_List file
from skim.quarkonium import ISRpipiccList
ISRpipicc = ISRpipiccList(path=ISRskimpath)

# output to Udst file
skimOutputUdst('ISRpipimumu.udst.root', ISRpipicc, path=ISRskimpath)

# print out Particle List statistics
summaryOfLists(ISRpipicc, path=ISRskimpath)

# output skim log information
setSkimLogging(skim_path=ISRskimpath)

# process the path
process(ISRskimpath)

# print out the summary
print(statistics)
