#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script demonstrates how to reconstruct semileptonic Btag using
# generically trained FEI+ Bsig with at least one lepton.
#
# Racha Cheaib (2017)
#
#####################################################
import sys
import glob
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from stdCharged import *
from skimExpertFunctions import *

gb2_setuprel = 'release-02-00-00'
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.WARNING, 'fei_database')

scriptName = sys.argv[0]
skimListName = scriptName[:-19]
skimCode = encodeSkimName(skimListName)
print(skimListName)
print(skimCode)


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>4')

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

import fei
particles = fei.get_default_channels(neutralB=True, chargedB=False, semileptonic=True, hadronic=False, KLong=False, removeSLD=True)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

# analysis_main.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

# now the FEI reconstruction is done
# and we're back in analysis_main pathB


from feiSLB0WithOneLep_List import *
BtagList = B0SLWithOneLep()
skimOutputUdst(skimCode, BtagList)
summaryOfLists(BtagList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
