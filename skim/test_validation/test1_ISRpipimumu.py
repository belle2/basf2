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
from stdCharged import *

fileList = ['../ISRpipimumu.dst.root']

inputMdstList('MC9', fileList)

# importing the reconstructed events from the ISRpipicc_List file
from skim.quarkonium import ISRpipiccList
ISRpipicc = ISRpipiccList()

# output to Udst file
skimOutputUdst('ISRpipimumu.udst.root', ISRpipicc)

# print out Particle List statistics
summaryOfLists(ISRpipicc)

# output skim log information
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
