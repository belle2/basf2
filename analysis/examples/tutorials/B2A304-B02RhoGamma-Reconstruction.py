#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> rho gamma
#       |
#       +-> pi+ pi-
#
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (Demeber 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdPhotons import stdPhotons
from stdCharged import stdLoosePi

# Run this tutorial either over signal MC or background MC (K*gamma)
# Add 10 signal MC files (each containing 1000 generated events)
filelistBKG = ['/group/belle2/tutorial/release_01-00-00/Bd_Kstgamma_GENSIMRECtoDST.dst.root']
filelistSIG = ['/group/belle2/tutorial/release_01-00-00/mdst_000001_prod00002326_task00000001.root']

# Run B0 -> rho gamma reconstruction over B0 -> rho gamma MC
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-SIGMC.root'
inputMdstList('default', filelistSIG)

# Run B0 -> rho gamma reconstruction over B0 -> K* gamma MC
# (uncomment next two lines and comment above two to run over BKG MC)
# rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-BKGMC.root'
# inputMdstList('default',filelistBKG)

# use standard final state particle lists
#
# creates "gamma:tight" ParticleList
stdPhotons('tight')

# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
reconstructDecay('rho0 -> pi+:loose pi-:loose', '0.6 < M < 1.0')

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
reconstructDecay('B0 -> rho0 gamma:tight', '5.2 < Mbc < 5.29 and abs(deltaE) < 2.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0')

from groups_of_varuables import event_variables, kinematic_variables, cluster_variables, \
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars

from modularAnalysis import variablesToNTuple
variablesToNTuple(filename=rootOutputFile,
                  decayString='B0',
                  treename='b0',
                  ['Mbc', 'deltaE'] +
                  event_variables +
                  kinematic_variables +
                  mc_variables +
                  convert_to_daughter_vars(kinematic_variables + cluster_variables + mc_variables, 0) +
                  convert_to_daughter_vars(kinematic_variables + cluster_variables + mc_variables, 1) +
                  convert_to_gd_vars(track_variables + pid_variables, 0, 0) +
                  convert_to_gd_vars(track_variables + pid_variables, 0, 1))


# Process the events
process(analysis_main)

# print out the summary
print(statistics)
