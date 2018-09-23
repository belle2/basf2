#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
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
from modularAnalysis import variablesToNtuple
import variableCollections as vc
from stdPi0s import *

# Add 10 signal MC files (each containing 1000 generated events)
# filelistSIG = \
#    ['/hsm/belle2/bdata/MC/signal/B2D0pi0/mcprod1405/BGx1/mc35_B2D0pi0_BGx1_s00/B2D0pi0_e0001r001*_s00_BGx1.mdst.root'
#     ]
filelistSIG = ['/group/belle2/tutorial/release_01-00-00/mdst-B0D0pi0.root']

inputMdstList('default', filelistSIG)

# use standard final state particle lists
#
# creates "pi0:looseFit" ParticleList
# https://confluence.desy.de/display/BI/Physics+StandardParticles
stdPi0s('looseFit')

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
reconstructDecay('D0:pi0pi0 -> pi0:looseFit pi0:looseFit', '1.7 < M < 2.0')

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
reconstructDecay('B0:all -> D0:pi0pi0 pi0:looseFit', '5.24 < Mbc < 5.29 and abs(deltaE) < 1.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0:all')


# Select variables that we want to store to ntuple
B0_vars = vc.event_meta_data + vc.inv_mass + vc.mc_truth + \
    vc.convert_to_all_selected_vars(
        vc.inv_mass + vc.mc_truth,
        'B0 -> ^D0 ^pi0') + \
    vc.convert_to_all_selected_vars(
        vc.cluster, 'B0 -> D0 [pi0 -> ^gamma ^gamma]')

pi0_vars = vc.mc_truth + vc.kinematics + vc.mass_before_fit + vc.event_meta_data + \
    ['extraInfo(BDT)', 'decayAngle(0)'] + \
    vc.convert_to_all_selected_vars(
        vc.cluster + vc.kinematics, 'pi0 -> ^gamma ^gamma')


# Saving variables to ntuple
output_file = 'B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.root'
variablesToNtuple('B0:all', B0_vars,
                  filename=output_file, treename='b0')
variablesToNtuple('pi0:looseFit', pi0_vars,
                  filename=output_file, treename='pi0')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
