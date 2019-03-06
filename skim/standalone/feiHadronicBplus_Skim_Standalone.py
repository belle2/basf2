#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
    FEI Hadronic B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180200
    fei training: default based, release-03-00-03 'FEIv4_2018_default_release_02_00_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling"]


from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *

gb2_setuprel = 'release-03-00-03'

import sys
import os
import glob
skimCode = encodeSkimName('feiHadronicBplus')
fileList = get_test_file("mixedBGx1", "MC11")


path = create_path()

inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforBplusHadronic(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)

# Apply final B+ tag cuts
BplushadronicList = BplusHadronic(path)
skimOutputUdst(skimCode, BplushadronicList, path=path)
summaryOfLists(BplushadronicList, path=path)

# Suppress noisy modules, and then process
setSkimLogging()
process(path)

# print out the summary
print(statistics)
