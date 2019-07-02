#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 and B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180300, 11180400
    fei training: MC11 based, release-03-02-00 'FEIv4_2018_MC11_release_03_00_00'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]

import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'

fileList = get_test_file("mixedBGx1", "MC12")
path = create_path()
inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection  cuts and FEI
runFEIforSLCombined(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B0 semileptonic tag cuts
B0semileptonicList = B0SL(path)
skimCode1 = encodeSkimName('feiSLB0')
skimOutputUdst(skimCode1, B0semileptonicList, path=path)
summaryOfLists(B0semileptonicList, path=path)

# Apply final B+ semileptonic tag cuts
BpsemileptonicList = BplusSL(path)
skimCode2 = encodeSkimName('feiSLBplus')
skimOutputUdst(skimCode2, BpsemileptonicList, path=path)
summaryOfLists(BpsemileptonicList, path=path)


setSkimLogging()
process(path)

# print out the summary
print(statistics)
