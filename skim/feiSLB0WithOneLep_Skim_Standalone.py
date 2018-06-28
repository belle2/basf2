#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI semi-leptonic (SL) B0 tag with one lepton skim list for
    generic analysis in WG1: (Semi-)Leptonic and Missing Energy
    Skim LFN code: 11180500
    Physics channels: (All available FEI B0 SL tags are reconstructed)

    *B0 -> D- e+ nu
    *B0 -> D- mu+ nu
    *B0 -> D+* e+ nu
    *B0 -> D+* mu+ nu
    *B0 -> anti-D0 pi- e+ nu
    *B0 -> anti-D0 pi- mu+ nu
    *B0 -> anti-D0* pi- e+ nu
    *B0 -> anti-D0* pi- mu+ nu
    In addition, all hadronic channels (listed in FEI B0 hadronic
    skim are used, where one of the hadronic D mesons is replaced by a
    SL D meson.

    Skimming script reconstructs SL Btag using generically trained
    FEI and Bsig with at least one lepton (e, mu). Signal side lepton
    is not stored in skim output.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
    Event precuts:
    R2EventLevel < 0.4
    nTracks > 4

    Tag side B:
    -5 < cosThetaBetweenParticleAndTrueB) < 3
    extraInfo(decayModeID) < 8 to remove semileptonic D channels
    sigProb > 0.005 to give < 10% retention

    Signal side:
    electron or muon from list 95eff
    B Mbc > 0
    """

__author__ = "R. Cheaib & S. Hollitt"

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

skimCode = encodeSkimName('feiSLB0WithOneLep')


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('default', fileList)

# Apply event cuts
applyEventCuts('R2EventLevel<0.4 and nTracks>4')

# Run FEI
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

# SL B0 skim
from feiSLB0WithOneLep_List import *
BtagList = B0SLWithOneLep()
skimOutputUdst(skimCode, BtagList)
summaryOfLists(BtagList)

# Suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
