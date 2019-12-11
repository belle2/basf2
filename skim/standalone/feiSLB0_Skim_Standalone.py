#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180300
    fei training: MC12 based, release-04-00-00 'FEIv4_2019_MC12_release_03_01_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
skimCode = expert.encodeSkimName('feiSLB0')

fileList = expert.get_test_file("MC12_mixedBGx1")

path = b2.create_path()

ma.inputMdstList('default', fileList, path=path)

from skim.fei import B0SL, runFEIforB0SL
# run pre-selection cuts and FEI
runFEIforB0SL(path)

# Include MC matching
path.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

# Apply final  B0 tag cuts
BtagList = B0SL(path)
expert.skimOutputUdst(skimCode, BtagList, path=path)
ma.summaryOfLists(BtagList, path=path)

# Suppress noisy modules, and then process
expert.setSkimLogging(path)
b2.process(path)

# print out the summary
print(b2.statistics)
