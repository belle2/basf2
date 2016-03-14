#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> D0 pi0
#       |
#       +-> pi0 pi0
#
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdFSParticles import stdPi0

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/hsm/belle2/bdata/MC/signal/B2D0pi0/mcprod1405/BGx1/mc35_B2D0pi0_BGx1_s00/B2D0pi0_e0001r001*_s00_BGx1.mdst.root'
     ]

inputMdstList(filelistSIG)

# use standard final state particle lists
#
# creates "pi0:all", "pi0:loose" and "pi0:good" ParticleLists
# pi0:all candidates are created form all good ECL clusters
# while pi0:loose and pi0:good have to pass loose and good cut
# on the BoostedDecisionTree output
stdPi0()

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
reconstructDecay('D0:pi0pi0 -> pi0:good pi0:good', '1.7 < M < 2.0')

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
reconstructDecay('B0:all -> D0:pi0pi0 pi0:good', '5.24 < Mbc < 5.29 and abs(deltaE) < 1.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0:all')

# create and fill flat Ntuple with MCTruth and kinematic information
toolsB0 = ['EventMetaData', '^B0']
toolsB0 += ['InvMass[BeforeFit]', 'B0 -> ^D0 ^pi0']
toolsB0 += ['DeltaEMbc', '^B0']
toolsB0 += ['Cluster', 'B0 -> D0 [pi0 -> ^gamma ^gamma]']
toolsB0 += ['MCTruth', '^B0 -> ^D0 ^pi0']

# create another set of tools for saving out all pi0 candidates
toolsPI0 = ['MCTruth', '^pi0 -> gamma gamma']
toolsPI0 += ['Kinematics', '^pi0 -> ^gamma ^gamma']
toolsPI0 += ['MassBeforeFit', '^pi0']
toolsPI0 += ['EventMetaData', '^pi0']
toolsPI0 += ['Cluster', 'pi0 -> ^gamma ^gamma']
toolsPI0 += ['CustomFloats[extraInfo(BDT):decayAngle(0)]', '^pi0']

# write out the flat ntuple
ntupleFile('B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.root')
ntupleTree('b0', 'B0:all', toolsB0)
ntupleTree('pi0', 'pi0:all', toolsPI0)

# Process the events
process(analysis_main)

# print out the summary
print statistics
