#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# This script reconstructs semileptonic Btags using
# generically trained FEI. Events must have a signal
# side lepton at 95% efficiency to pass skim.
#
# FEIv4_2018_MC9_release_02_00_00
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

fileList =\
    ['/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000273/s00/e0000/4S/r00000/signal/sub00/*'
     ]
inputMdstList('MC9', fileList)

applyEventCuts('R2EventLevel<0.4 and nTracks>=4')

from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')
# Weightfiles for FEIv4_2018_MC9_release_02_00_00 in this database

import fei
particles = fei.get_default_channels(chargedB=True, neutralB=True, hadronic=False, semileptonic=True, KLong=False, removeSLD=True)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_00', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)


from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')

from stdCharged import *
stdE('95eff')
stdMu('95eff')

reconstructDecay('B0:sig1 -> e+:95eff', 'Mbc>0', 1)
reconstructDecay('B0:sig2 -> mu+:95eff', 'Mbc>0', 2)
reconstructDecay('B0:sig3 -> e-:95eff', 'Mbc>0', 3)
reconstructDecay('B0:sig4 -> mu-:95eff', 'Mbc>0', 4)
copyLists('B0:all', ['B0:sig1', 'B0:sig2', 'B0:sig3', 'B0:sig4'])

applyCuts('B0:semileptonic', '-5<cosThetaBetweenParticleAndTrueB<3 and sigProb>0.005 and extraInfo(decayModeID)<8')
reconstructDecay('Upsilon(4S):sigB0 -> anti-B0:semileptonic B0:all', '')
applyCuts('B0:semileptonic', 'nParticlesInList(Upsilon(4S):sigB0)>0')
B0semileptonicList = ['B0:semileptonic']

reconstructDecay('B+:sig1 -> e+:95eff', 'Mbc>0', 1)
reconstructDecay('B+:sig2 -> mu+:95eff', 'Mbc>0', 2)
reconstructDecay('B+:sig3 -> e-:95eff', 'Mbc>0', 3)
reconstructDecay('B+:sig4 -> mu-:95eff', 'Mbc>0', 4)
copyLists('B+:all', ['B+:sig1', 'B+:sig2', 'B+:sig3', 'B+:sig4'])

applyCuts('B+:semileptonic', '-5<cosThetaBetweenParticleAndTrueB<3 and sigProb>0.009 and extraInfo(decayModeID)<8')
reconstructDecay('Upsilon(4S):sigBP -> B-:semileptonic B+:all', '')
applyCuts('B+:semileptonic', 'nParticlesInList(Upsilon(4S):sigBP)>0')
BpsemileptonicList = ['B+:semileptonic']


skimCode1 = encodeSkimName('feiSLB0WithOneLep')
skimOutputUdst(skimCode1, B0semileptonicList)
summaryOfLists(B0semileptonicList)

skimCode2 = encodeSkimName('feiSLBplusWithOneLep')
skimOutputUdst(skimCode2, BpsemileptonicList)
summaryOfLists(BpsemileptonicList)


setSkimLogging()

process(analysis_main)

# print out the summary
print(statistics)
