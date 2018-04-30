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
use_central_database('production', LogLevel.WARNING, 'fei_database')

fileList =\
    ['/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000273/s00/e0000/4S/r00000/signal/sub00/*'
     ]
inputMdstList('default', fileList)

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B0:generic', looseMCMatching=True)


from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')

applyCuts('B0:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')
B0hadronicList = ['B0:generic']


applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')
BphadronicList = ['B+:generic']


outputLFN1 = getOutputLFN('feiHadronicB0')
skimOutputUdst(outputLFN1, B0hadronicList)
summaryOfLists(B0hadronicList)

outputLFN2 = getOutputLFN('feiHadronicBplus')
skimOutputUdst(outputLFN2, BphadronicList)
summaryOfLists(BphadronicList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
    if module.type() == "MCMatcher":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
