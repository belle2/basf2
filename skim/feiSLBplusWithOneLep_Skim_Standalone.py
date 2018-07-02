#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script demonstrates how to reconstruct  semileptonic Btag using
# generically trained FEI with Bsig to at least one lepton.
#
# Racha Cheaib (2017)
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

skimCode = encodeSkimName('feiSLBplusWithOneLep')


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]


inputMdstList('MC9', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>4')

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

import fei
particles = fei.get_MC9_channels(chargedB=True, neutralB=False, semileptonic=True, hadronic=False, KLong=False, removeSLD=True)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

# now the FEI reconstruction is done
# and we're back in analysis_main pathB

from feiSLBplusWithOneLep_List import *
BtagList = BplusSLWithOneLep()

skimOutputUdst(skimCode, BtagList)
summaryOfLists(BtagList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
