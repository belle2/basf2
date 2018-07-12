#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script reconstructs hadronic Btags using
# generically trained FEI.
#
# FEIv4_2018_MC9_2
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
gb2_setuprel = 'release-02-00-00'

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('MC9', fileList)

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')
# Weightfiles for FEIv4_2018_MC9_2 in this database

import fei
particles = fei.get_default_channels(chargedB=True, neutralB=True, hadronic=True, semileptonic=False, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)


from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')

applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')
B0hadronicList = ['B0:generic']


applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')
BphadronicList = ['B+:generic']


skimCode1 = encodeSkimName('feiHadronicB0')
skimOutputUdst(skimCode1, B0hadronicList)
summaryOfLists(B0hadronicList)

skimCode2 = encodeSkimName('feiHadronicBplus')
skimOutputUdst(skimCode2, BphadronicList)
summaryOfLists(BphadronicList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
    if module.type() == "MCMatcher":
        module.set_log_level(LogLevel.ERROR)

process(analysis_main)

# print out the summary
print(statistics)
