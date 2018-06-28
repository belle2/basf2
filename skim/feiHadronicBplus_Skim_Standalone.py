#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
    FEI Hadronic B+ tag skim standalone for generic analysis in WG1:
    (Semi-)Leptonic and Missing Energy
    Skim LFN code: 11180200
    Physics channels: (All available FEI B+ Hadronic tags are
    reconstructed)
    *B+ -> anti-D0 pi+
    *B+ -> anti-D0 pi+ pi0
    *B+ -> anti-D0 pi+ pi0 pi0
    *B+ -> anti-D0 pi+ pi+ pi-
    *B+ -> anti-D0 pi+ pi+ pi- pi0
    *B+ -> anti-D0 D+
    *B+ -> anti-D0 D+ KS0
    *B+ -> anti-D0* D+ KS0
    *B+ -> anti-D0 D+* KS0
    *B+ -> anti-D0* D+* KS0
    *B+ -> anti-D0 D0 K+
    *B+ -> anti-D0* D0 K+
    *B+ -> anti-D0 D*(2010)0 K+
    *B+ -> anti-D0* D*(2010)0 K+
    *B+ -> Ds+ anti-D0
    *B+ -> anti-D0* pi+
    *B+ -> anti-D0* pi+ pi0
    *B+ -> anti-D0* pi+ pi0 pi0
    *B+ -> anti-D0* pi+ pi+ pi-
    *B+ -> anti-D0* pi+ pi+ pi- pi0
    *B+ -> Ds+* anti-D0
    *B+ -> Ds+ anti-D0*
    *B+ -> anti-D0 K+
    *B+ -> D- pi+ pi+
    *B+ -> D- pi+ pi+ pi0
    *B+ -> J/Psi K+
    *B+ -> J/Psi K+ pi+ pi-
    *B+ -> J/Psi K+ pi0
    *B+ -> J/Psi KS0 pi+

    Skimming script reconstructs hadronic Btag using generically trained
    FEI.

    Skim Liasons: S. Hollitt & H. Wakeling

    Cuts applied are::
        Event precuts:
        R2EventLevel < 0.4
        nTracks > 4 

        Tag side B:
        Mbc > 5.24
        abs(deltaE) < 0.200
        sigProb > 0.001
    """

__author__ = "R. Cheaib & S. Hollitt"


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

inputMdstList('default', fileList)

# Apply event cuts
applyEventCuts('R2EventLevel<0.4 and nTracks>4')

# Run FEI
from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()

import fei
particles = fei.get_default_channels(neutralB=False, chargedB=True, hadronic=True, semileptonic=False, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_2', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)

analysis_main.add_module('MCMatcherParticles', listName='B+:generic', looseMCMatching=True)

# Hadronic Bplus skim
from feiHadronicBplus_List import*
BplushadronicList = BplusHadronic()

skimOutputUdst(skimCode, BplushadronicList)
summaryOfLists(BplushadronicList)

# Suppress noisy modules, and then process
setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
