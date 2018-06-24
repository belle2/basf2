#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script reconstructs hadronic Btag using
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

gb2_setuprel = 'release-02-00-00'
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.WARNING, 'fei_database')

import sys
import os
import glob
skimCode = encodeSkimName('feiHadronicBplus')
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>4')

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

import fei
particles = fei.get_MC9_channels(neutralB=False, chargedB=True, hadronic=True, semileptonic=False, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)


from feiHadronicBplus_List import*
BplushadronicList = BplusHadronic()


skimOutputUdst(skimCode, BplushadronicList)
summaryOfLists(BplushadronicList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
