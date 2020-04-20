#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI Hadronic B0 tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180100
    fei training: MC13 based, release-04-01-01 'FEIv4_2020_MC13_release_04_01_01'
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling", "Phil Grace"]

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-01-01'
skimCode = expert.encodeSkimName('feiHadronicB0')
fileList = expert.get_test_file("MC12_mixedBGx1")

path = b2.create_path()

ma.inputMdstList('default', fileList, path=path)

from skim.fei import B0Hadronic, runFEIforB0Hadronic
# run pre-selection cuts and FEI
path2 = runFEIforB0Hadronic(path)

# Include MC matching
path2.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)

# Apply final B0 tag cuts
B0HadronicList = B0Hadronic(path)
expert.skimOutputUdst(skimCode, B0HadronicList, path=path2)
ma.summaryOfLists(B0HadronicList, path=path2)

# Suppress noisy modules, and then process
expert.setSkimLogging(path2, ['ParticleCombiner'])
b2.process(path)

# print out the summary
print(b2.statistics)
