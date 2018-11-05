#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script reconstructs hadronic Btags using
# generically trained FEI.
#
# FEIv4_2018_MC9_release_02_00_01
#
#####################################################
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
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('MC9', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>=4')

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')
# Weightfiles for FEIv4_2018_MC9_release_02_00_01 in this database

import fei
particles = fei.get_default_channels(chargedB=True, neutralB=True, hadronic=True, semileptonic=False, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_01', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)


from skim.fei import *
B0hadronicList = B0hadronic()
skimCode1 = encodeSkimName('feiHadronicB0')
skimOutputUdst(skimCode1, B0hadronicList)
summaryOfLists(B0hadronicList)

from skim.fei import*
BplushadronicList = BplusHadronic()
skimCode2 = encodeSkimName('feiHadronicBplus')
skimOutputUdst(skimCode2, BplushadronicList)
summaryOfLists(BplushadronicList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
    if module.type() == "MCMatcher":
        module.set_log_level(LogLevel.ERROR)

process(analysis_main)

# print out the summary
print(statistics)
