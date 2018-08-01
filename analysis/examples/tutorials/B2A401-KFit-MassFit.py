#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform mass fit with
# the KFit. In this example the following decay chain:
#
# B0 -> D0 pi0
#       |
#       +-> pi0 pi0
#
# is reconstructed. The mass fits is performed on D0
# candidates (as well as pi0 candidates by default).
#
# Note: This example is build upon
# B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.py
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
from modularAnalysis import massKFit
from stdPi0s import stdPi0s

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = ['/group/belle2/tutorial/release_01-00-00/mdst-B0D0pi0.root']

inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi0:looseFit" ParticleList
stdPi0s('looseFit')

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
reconstructDecay('D0:pi0pi0 -> pi0:looseFit pi0:looseFit', '1.7 < M < 2.0')

# perform mass fit using KFit
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
massKFit('D0:pi0pi0', 0.0)

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
reconstructDecay('B0:all -> D0:pi0pi0 pi0:looseFit', '5.24 < Mbc < 5.29 and abs(deltaE) < 1.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0:all')

# create and fill flat Ntuple with MCTruth and kinematic information
from variableCollections import event_variables, kinematic_variables, cluster_variables,\
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars\

from modularAnalysis import variablesToNTuple
output_file = 'B2A401-KFit-MassFit.root'
variablesToNTuple(filename=output_file,
                  decayString='B0:all',
                  treename='b0',
                  ['chiProb', 'Mbc', 'deltaE'] +
                  event_variables + kinematic_variables + mc_variables +
                  convert_to_daughter_vars(kinematic_variables + mc_variables, 0) +
                  convert_to_daughter_vars(kinematic_variables + mc_variables, 1) +
                  convert_to_gd_vars(cluster_variables, 1, 0) +
                  convert_to_gd_vars(cluster_variables, 1, 1))
variablesToNTuple(filename=output_file,
                  decayString='pi0:looseFit',
                  treename='pi0',
                  ['extraInfo(BDT)', 'decayAngle(0)'] +
                  event_variables + kinematic_variables + mc_variables +
                  convert_to_daughter_vars(kinematic_variables + cluster_variables + mc_variables, 0) +
                  convert_to_daughter_vars(kinematic_variables + cluster_variables + mc_variables, 1))


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
