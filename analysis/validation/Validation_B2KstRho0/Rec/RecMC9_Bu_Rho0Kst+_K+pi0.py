#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from modularAnalysis import inputMdst, inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import fillParticleList, fillParticleListFromMC
from modularAnalysis import fillConvertedPhotonsList
from modularAnalysis import loadGearbox
from modularAnalysis import vertexKFit, massKFit, massVertexKFit
from modularAnalysis import vertexRave, massVertexRave
from modularAnalysis import rankByHighest
from modularAnalysis import copyParticles
from modularAnalysis import printVariableValues, variablesToExtraInfo, applyCuts
from modularAnalysis import buildRestOfEvent, buildContinuumSuppression, appendROEMasks
import stdPi0s
import stdCharged

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [5])

filelist = []

rootOutputFile = "Bu_Rho0Kst+_K+pi0.root"
inputMdstList('default', filelist)

gb2_setuprel = "release-00-09-01"

stdPi0s.stdPi0s('loose')
stdCharged.stdPi('all')
stdCharged.stdK('all')


nDaugCut = 'countDaughters( ) == 2'
pdgDaugCut = 'countDaughters(abs(mcPDG) == 113) == 1 and countDaughters(abs(mcPDG) == 323) == 1'
rhoCutCase1 = '[daughter(0,abs(mcPDG)) == 113 and daughter(0,countDaughters( ))== 2\
 and daughter(0,countDaughters(abs(mcPDG) == 211)) == 2]'
rhoCutCase2 = '[daughter(1,abs(mcPDG)) == 113 and daughter(1,countDaughters( ))== 2\
 and daughter(1,countDaughters(abs(mcPDG) == 211)) == 2]'
KstCutCase1 = '[daughter(0,abs(mcPDG)) == 323 and daughter(0,countDaughters( ))== 2\
 and daughter(0,countDaughters(abs(mcPDG) == 111)) == 1 and daughter(0,countDaughters(abs(mcPDG) == 321)) == 1]'
KstCutCase2 = '[daughter(1,abs(mcPDG)) == 323 and daughter(1,countDaughters( ))== 2\
 and daughter(1,countDaughters(abs(mcPDG) == 111)) == 1 and daughter(1,countDaughters(abs(mcPDG) == 321)) == 1]'
MCCut = nDaugCut + ' and ' + pdgDaugCut + ' and [' + KstCutCase1 + ' or ' + \
    KstCutCase2 + '] and [' + rhoCutCase1 + ' or ' + rhoCutCase2 + ']'
#
fillParticleListFromMC('B+:gen', MCCut, True)

from aliases import variables
variables.addAlias('MCTrue', 'passesCut(' + MCCut + ')')
variables.addAlias('gd0_isSignal', 'daughter(0,daughter(1,daughter(0,isSignal)))')
variables.addAlias('gd1_isSignal', 'daughter(0,daughter(1,daughter(1,isSignal)))')
variables.addAlias('Mbc_corr', 'Mbc')

copyParticles('pi0:my', 'pi0:loose', False)
massVertexKFit('pi0:my', 0.0, '')
matchMCTruth('pi0:my')
reconstructDecay('rho0:pipi -> pi+:all pi-:all', '0.52 < M < 1.05')
reconstructDecay('K*+:Kpi0 -> K+:all pi0:my', '0.792 < M < 0.992')
reconstructDecay('B+:ours -> K*+:Kpi0 rho0:pipi', '4.8 < M < 5.5')
massVertexRave('B+:ours', 0.0, 'B+ -> [K*+ -> ^K+ ^pi0] [rho0 -> ^pi+ ^pi-] ')

matchMCTruth('B+:ours')
buildRestOfEvent('B+:ours')
cleanMask = ('cleanMask', 'useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
appendROEMasks('B+:ours', [cleanMask])
buildContinuumSuppression('B+:ours', 'cleanMask')
rankByHighest("B+:ours", "chiProb")

variablesToExtraInfo('B+:ours', {'random': 'random_val'})
rankByHighest("B+:ours", "rnd")

matchMCTruth('B+:gen')

from BTools import RecBTools, RoETools, MCBTools, pi0Tools, MCpi0Tools

Tools = RecBTools + RoETools + MCBTools + pi0Tools + MCpi0Tools
BGenTools = RecBTools + pi0Tools

ntupleFile(rootOutputFile)
ntupleTree('B2KstRho', 'B+:ours', Tools)
ntupleTree('MCB2KstRho', 'B+:gen', BGenTools)

# progress
progress = register_module('Progress')
analysis_main.add_module(progress)

process(analysis_main)

# Print call statistics
print(statistics)
