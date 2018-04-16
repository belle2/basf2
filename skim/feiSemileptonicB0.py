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
from analysisPath import analysis_main
from beamparameters import add_beamparameters


gb2_setuprel = 'release-01-00-00'

use_central_database('production', LogLevel.WARNING, 'fei_database')

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()


import fei
particles = fei.get_default_channels()
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2017_MC7_Track14_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

# now the FEI reconstruction is done
# and we're back in analysis_main pathB

# apply some very loose cuts to reduce the number
# of Btag candidates

applyCuts('B0:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and extraInfo(decayModeID)<8')

# rank Btag canidates according to their SignalProbability


B0SemiLeptonicList = ['B0:semileptonic']


skimOutputUdst('feiSemiLeptonicB0', B0SemiLeptonicList)
summaryOfLists(B0SemiLeptonicList)


process(analysis_main)

# print out the summary
print(statistics)
