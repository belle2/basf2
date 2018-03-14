#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# B meson decay with missing energy, e.g. :
#
# Y(4S) -> Btag- Bsig+
#                 |
#                 +-> mu+ nu
#
# First the Btag candidate is reconstructed in the
# following decay chain:
#
# Btag- -> D0 pi-
#          |
#          +-> K- pi+
#          +-> K- pi+ pi0
#          +-> K- pi+ pi+ pi-
#          +-> K- K+
#          +-> pi- pi+
#
# The signature of signal B meson decay is single muon,
# therefore by reconstructing the muon one already reconstructs
# the signal B.
#
# Once, Btag and Bsig candidates are reconstructed the
# RestOfEvent is filled for each BtagBsig combination
# with the remaining Tracks, ECLClusters and KLMClusters
# that are not used to reconstruct Btag or Bsig. The
# RestOfEvent object is then used as an input for E_extra,
# MissingMass^2, etc. variables.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import copyLists
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import buildRestOfEvent
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdCharged import *
from stdFSParticles import stdPi0s

# load data
inputMdst('default', 'B2A101-Y4SEventGeneration-gsim-BKGx1.root')

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()
# creates "mu+:loose" ParticleList (and c.c.)
stdLooseMu()

# creates "pi0:looseFit" ParticleList
stdPi0s('looseFit')

# 1. reconstruct D0 in multiple decay modes
reconstructDecay('D0:ch1 -> K-:loose pi+:loose', '1.8 < M < 1.9', 1)
reconstructDecay('D0:ch2 -> K-:loose pi+:loose pi0:looseFit', '1.8 < M < 1.9', 2)
reconstructDecay('D0:ch3 -> K-:loose pi+:loose pi+:loose pi-:loose', '1.8 < M < 1.9', 3)
reconstructDecay('D0:ch4 -> K-:loose K+:loose', '1.8 < M < 1.9', 4)
reconstructDecay('D0:ch5 -> pi-:loose pi+:loose', '1.8 < M < 1.9', 5)

# merge the D0 lists together into one single list
copyLists('D0:all', ['D0:ch1', 'D0:ch2', 'D0:ch3', 'D0:ch4', 'D0:ch5'])

# 2. reconstruct Btag+ -> anti-D0 pi+
reconstructDecay('B+:tag -> anti-D0:all pi+:loose', '5.2 < Mbc < 5.29 and abs(deltaE) < 1.0', 1)
matchMCTruth('B+:tag')

# 3. reconstruct Upsilon(4S) -> Btag+ Bsig- -> Btag+ mu-
reconstructDecay('Upsilon(4S) -> B-:tag mu+:loose', "")

# perform MC matching (MC truth asociation)
matchMCTruth('Upsilon(4S)')

# 5. build rest of the event
buildRestOfEvent('Upsilon(4S)')

# 6. Dump info to ntuple
toolsBTAG = ['MCTruth', '^B- -> ^D0 pi-']
toolsBTAG += ['Kinematics', 'B- -> ^D0 pi-']
toolsBTAG += ['DeltaEMbc', '^B-']
toolsBTAG += ['InvMass', 'B- -> ^D0 pi-']
toolsBTAG += ['CustomFloats[extraInfo(decayModeID)]', '^B- -> ^D0 pi-']

tools4S = ['MCTruth', '^Upsilon(4S) -> ^B- ^mu+']
tools4S += ['DeltaEMbc', 'Upsilon(4S) -> ^B- mu+']
tools4S += ['ROEMultiplicities', '^Upsilon(4S)']
tools4S += ['RecoilKinematics', '^Upsilon(4S)']
tools4S += ['ExtraEnergy', '^Upsilon(4S)']
tools4S += ['Kinematics', '^Upsilon(4S) -> [B- -> ^D0 pi-] mu+']
tools4S += ['InvMass', 'Upsilon(4S) -> [B- -> ^D0 pi-] mu+']
tools4S += ['CustomFloats[extraInfo(decayModeID)]', 'Upsilon(4S) -> [^B- -> ^D0 pi-] mu+']

ntupleFile('B2A305-Btag+SingleMuon-Reconstruction.root')
ntupleTree('btag', 'B-:tag', toolsBTAG)
ntupleTree('btagbsig', 'Upsilon(4S)', tools4S)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
