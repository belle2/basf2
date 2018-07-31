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
from stdPi0s import stdPi0s

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

from groups_of_varuables import event_variables, kinematic_variables, cluster_variables, \
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars,\
    roe_multiplicities, recoil_kinematics

from modularAnalysis import variablesToNTuple
variablesToNTuple(filename=rootOutputFile,
                  decayString='B-:tag',
                  treename='btag',
                  ['Mbc', 'deltaE', 'extraInfo(decayModeID)'] +
                  event_variables +
                  kinematic_variables +
                  mc_variables +
                  convert_to_daughter_vars(['extraInfo(decayModeID)'] + kinematic_variables + mc_variables, 0))

variablesToNTuple(filename=rootOutputFile,
                  decayString='Upsilon(4S)',
                  treename='btagbsig',
                  roe_multiplicities +
                  recoil_kinematics +
                  event_variables +
                  kinematic_variables +
                  mc_variables +
                  convert_to_daughter_vars(
                      ['Mbc', 'deltaE', 'extraInfo(decayModeID)'] +
                      kinematic_variables +
                      mc_variables, 0) +
                  convert_to_daughter_vars(
                      ['extraInfo(decayModeID)'] +
                      kinematic_variables +
                      mc_variables, 1) +
                  convert_to_gd_vars(
                      ['extraInfo(decayModeID)'] +
                      kinematic_variables, 0, 0))

#

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
