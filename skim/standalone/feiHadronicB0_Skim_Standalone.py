#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI Hadronic B0 tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180100
    fei training: MC9 based, release-02-00-01 'FEIv4_2018_MC9_release_02_00_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling"]


import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *

gb2_setuprel = 'release-02-00-01'
skimCode = encodeSkimName('feiHadronicB0')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

path = create_path()

inputMdstList('MC9', fileList, path=path)

from skim.fei import *
# run pre-selection cuts and FEI
runFEIforB0Hadronic(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)

# Apply final B0 tag cuts
B0hadronicList = B0hadronic(path)
skimOutputUdst(skimCode, B0hadronicList, path=path)
summaryOfLists(B0hadronicList, path=path)

# Suppress noisy modules, and then process
setSkimLogging()
process(path)

# print out the summary
print(statistics)
