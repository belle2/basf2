#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# This tutorial demonstrates how to apply ModeSelector to FEI B meson    #
# candidates for improved signal probability estimation.                  #
#                                                                        #
# ModeSelector is a neural network that considers information from all   #
# B candidates in an event to provide a better signal probability score. #
#                                                                        #
##########################################################################

# import sys
#
# sys.path.insert(0, '/home/pf/basf2/analysis/scripts')
import basf2 as b2
import modeSelector
import modularAnalysis as ma
from stdPi0s import stdPi0s

# Set up logging
b2.set_log_level(b2.LogLevel.INFO)

# Create path
my_path = b2.create_path()

# Input file (should be FEI output with B meson candidates)
# For this example, we assume a file with B+:feiHadronic and B0:feiHadronic lists
ma.inputMdstList(
    filelist=['/home/pf/dataframes/MC16rd_skim/udst_000001_prod00051442_task230000001.root'],  # Replace with your input file
    path=my_path
)

# Prepend the analysis globaltag for accessing payloads
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Create standard pi0 list needed for D* veto reconstruction
stdPi0s('eff50_May2020Fit', path=my_path)

# Define the particle lists to process
particle_lists = ['B+:feiHadronic', 'B0:feiHadronic']

# Option 1: Use ModeSelector with D* veto reconstruction
# This provides better discrimination but requires more computation
for plist in particle_lists:
    modeSelector.addDstarVeto(plist, path=my_path)

# modeSelector.modeSelector(
#     particleLists=particle_lists,
#     output_variable='BplusScore',
#     path=my_path
# )

# Option 2: Use ModeSelector with local model files (for testing/development)
# Uncomment the following to use local ONNX models instead of database payloads:
#
modeSelector.modeSelector(
    particleLists=particle_lists,
    cat_model_path='modeSelector_test/cat_model.onnx',
    main_model_path='modeSelector_test/main_model.onnx',
    has_inputs_path='modeSelector_test/has_inputs.txt',
    output_variable='BplusScore',
    path=my_path
)

# Define output variables
output_variables = [
    # Basic kinematics
    'Mbc', 'deltaE', 'M',
    # FEI signal probability
    'extraInfo(SignalProbability)',
    'extraInfo(decayModeID)',
    # ModeSelector output
    'extraInfo(BplusScore)',
    'extraInfo(BplusScore_catB0)',
    'extraInfo(BplusScore_catBp)',
    'extraInfo(BplusScore_catCont)',
    # D* veto variables (if addDstarVeto was used)
    'extraInfo(Dstp_deltaMassDiff)',
    'extraInfo(Dstp_chiProb)',
    'extraInfo(Dst0_deltaMassDiff)',
    'extraInfo(Dst0_chiProb)',
]

# Write output ntuple for B+ candidates
ma.variablesToNtuple(
    'B+:feiHadronic',
    variables=output_variables,
    filename='modeSelector_output_Bp.root',
    treename='Bp',
    path=my_path
)

# Write output ntuple for B0 candidates
ma.variablesToNtuple(
    'B0:feiHadronic',
    variables=output_variables,
    filename='modeSelector_output_B0.root',
    treename='B0',
    path=my_path
)

# Process events
b2.process(my_path)

# Print statistics
print(b2.statistics)
