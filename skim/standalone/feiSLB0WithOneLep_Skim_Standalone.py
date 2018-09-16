#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""FEI Semi-leptonic B0 tag skim standalone for generic analysis in the
    (Semi-)Leptonic and Missing Energy Working Group
    Skim LFN code: 11180300
    """

__authors__ = ["Racha Cheaib", "Sophie Hollitt", "Hannah Wakeling"]

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
skimCode = encodeSkimName('feiSLB0WithOneLep')


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>=4')

# Run FEI
from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')

import fei
particles = fei.get_default_channels(neutralB=True, chargedB=False, semileptonic=True, hadronic=False, KLong=False, removeSLD=True)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_00', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

# analysis_main.add_module('MCMatcherParticles', listName='B0:semileptonic', looseMCMatching=True)

# now the FEI reconstruction is done
# and we're back in analysis_main pathB

# SL B0 skim
from skim.fei import B0SLWithOneLep
BtagList = B0SLWithOneLep()
skimOutputUdst(skimCode, BtagList)
summaryOfLists(BtagList)

# Suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
