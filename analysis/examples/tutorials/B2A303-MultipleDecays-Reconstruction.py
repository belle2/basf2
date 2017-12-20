#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following decay chain involving several decay modes:
#
# B- -> D0 pi-
#       |
#       +-> K- pi+
#       +-> K- pi+ pi0
#       +-> K- pi+ pi+
#       +-> K- K+
#       +-> pi- pi+
#
# Note: This reconstruction is performed on generated level
# to speed up the reconstruction during the tutorial. However,
# the reconstruction can as well be performed using reconstructed
# final state particles.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleListsFromMC
from modularAnalysis import reconstructDecay
from modularAnalysis import copyLists
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree

# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    sys.exit('Required input file (B2A101-Y4SEventGeneration-evtgen.root) does not exist. '
             'Please run B2A101-Y4SEventGeneration.py tutorial script first.')

# load input ROOT file
inputMdst('None', 'B2A101-Y4SEventGeneration-evtgen.root')

# create and fill final state ParticleLists
kaons = ('K-', '')
pions = ('pi-', '')
photons = ('gamma', '')
fillParticleListsFromMC([kaons, pions, photons])

# 1. reconstruct pi0 -> gamma gamma decay
# InvM is the sum of daughter momenta whereas M is the initialised PDG mass ...
reconstructDecay('pi0 -> gamma gamma', '0.1 < InvM < 0.15')

# 2. reconstruct D0 in multiple decay modes
reconstructDecay('D0:ch1 -> K- pi+', '1.8 < M < 1.9', 1)
reconstructDecay('D0:ch2 -> K- pi+ pi0', '1.8 < M < 1.9', 2)
reconstructDecay('D0:ch3 -> K- pi+ pi+ pi-', '1.8 < M < 1.9', 3)
reconstructDecay('D0:ch4 -> K- K+', '1.8 < M < 1.9', 4)
reconstructDecay('D0:ch5 -> pi+ pi-', '1.8 < M < 1.9', 5)

# merge the D0 lists together into one single list
copyLists('D0:all', ['D0:ch1', 'D0:ch2', 'D0:ch3', 'D0:ch4', 'D0:ch5'])

# 3. reconstruct B+ -> anti-D0 pi+ decay
reconstructDecay('B+:D0pi -> anti-D0:all pi+', '5.24 < Mbc < 5.29 and abs(deltaE) < 1.0', 1)

# perform MC matching (MC truth asociation)
matchMCTruth('B+:D0pi')

# create and fill flat Ntuple with MCTruth and kinematic information
toolsB = ['EventMetaData', '^B+']
toolsB += ['DeltaEMbc', '^B+']
toolsB += ['MCTruth', '^B+']
toolsB += ['InvMass', 'B+ -> ^anti-D0 pi+']
toolsB += ['CustomFloats[extraInfo(decayModeID)]', 'B+ -> ^anti-D0 pi+']

# write out the flat ntuple
ntupleFile('B2A303-MultipleDecays-Reconstruction.root')
ntupleTree('bp', 'B+:D0pi', toolsB)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
