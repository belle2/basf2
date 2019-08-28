#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180300
    fei training: MC12 based, release-03-02-00 'FEIv4_2019_MC12_release_03_01_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]

import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'
skimCode = encodeSkimName('feiSLB0')

fileList = get_test_file("mixedBGx1", "MC12")

path = create_path()

inputMdstList('default', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforB0SL(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

# Apply final  B0 tag cuts
BtagList = B0SL(path)
skimOutputUdst(skimCode, BtagList, path=path)
summaryOfLists(BtagList, path=path)

# Suppress noisy modules, and then process
setSkimLogging(path)
process(path)

# print out the summary
print(statistics)
