#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# Skimming script reconstructs hadronic Btag using
# generically trained FEI.
#
#
#
#####################################################
import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)

gb2_setuprel = 'release-02-00-00'
scriptName = sys.argv[0]
skimListName = scriptName[:-19]
skimCode = encodeSkimName(skimListName)
print(skimListName)
print(skimCode)


use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.WARNING, 'fei_database')
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>4')

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

import fei
particles = fei.get_default_channels(neutralB=True, chargedB=False, hadronic=True, semileptonic=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)

from feiHadronicB0_List import *
B0hadronicList = B0hadronic()
skimOutputUdst(skimCode, B0hadronicList)
summaryOfLists(B0hadronicList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
    if module.type() == "MCMatcher":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
