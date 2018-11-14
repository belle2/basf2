#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 and B+ tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180300, 11180400
    fei training: MC9 based, release-02-00-01 'FEIv4_2018_MC9_release_02_00_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling"]

import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *

gb2_setuprel = 'release-02-00-01'

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/s\
ub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

path = create_path()
inputMdstList('MC9', fileList, path=path)

from skim.fei import *
# run pre-selection  cuts and FEI
runFEIforSLWithOneLepCombined(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)
path.add_module('MCMatcherParticles', listName='B+:semileptonic', looseMCMatching=True)

# Apply final B0 semileptonic tag cuts
B0semileptonicList = B0SLWithOneLep(path)
skimCode1 = encodeSkimName('feiSLB0WithOneLep')
skimOutputUdst(skimCode1, B0semileptonicList, path=path)
summaryOfLists(B0semileptonicList, path=path)

# Apply final B+ semileptonic tag cuts
BpsemileptonicList = BplusSLWithOneLep(path)
skimCode2 = encodeSkimName('feiSLBplusWithOneLep')
skimOutputUdst(skimCode2, BpsemileptonicList, path=path)
summaryOfLists(BpsemileptonicList, path=path)


setSkimLogging()
process(path)

# print out the summary
print(statistics)
