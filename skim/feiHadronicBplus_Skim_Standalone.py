#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script demonstrates how to reconstruct Btag using
# generically trained FEI.
#
#
#
#####################################################

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *
set_log_level(LogLevel.INFO)

gb2_setuprel = 'release-02-00-00'

import sys
import os
import glob
scriptName = sys.argv[0]
skimListName = scriptName[:-19]
outputLFN = getOutputLFN(skimListName)
print(skimListName)
print(outputLFN)
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('default', fileList)

use_central_database('production', LogLevel.WARNING, 'fei_database')
from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)


from feiHadronicBplus_List import*
BplushadronicList = BplusHadronic()


skimOutputUdst(outputLFN, BplushadronicList)
summaryOfLists(BplushadronicList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)

for module in analysis_main.modules():
    if module.type() == "MCMatcher":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
