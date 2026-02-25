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
# This tutorial demonstrates how to apply ModeSelector to FEI B meson   #
# candidates for improved signal probability estimation.                 #
#                                                                        #
# Usage:                                                                 #
#   basf2 applyModeSelector.py -- [options]                              #
#                                                                        #
# Output files:                                                          #
#   <output>_Bp.root  - B+ candidates ntuple                            #
#   <output>_B0.root  - B0 candidates ntuple                            #
#                                                                        #
##########################################################################

import argparse

import basf2 as b2
import modeSelector
import modularAnalysis as ma

parser = argparse.ArgumentParser()
parser.add_argument('--input', nargs='+',
                    default=['/home/pf/dataframes/MC16rd_skim/udst_000001_prod00051442_task230000001.root'],
                    help='Input ROOT file(s) with FEI B meson candidates')
parser.add_argument('--output', default='modeSelector_output',
                    help='Output prefix for ntuples (default: modeSelector_output)')
parser.add_argument('--cat-model', default='modeSelector_test/cat_model.onnx',
                    help='Path to category ONNX model')
parser.add_argument('--main-model', default='modeSelector_test/main_model.onnx',
                    help='Path to main ONNX model')
args = parser.parse_args()

# Set up logging
b2.set_log_level(b2.LogLevel.INFO)

# Create path
my_path = b2.create_path()

ma.inputMdstList(
    filelist=args.input,
    path=my_path
)

# Prepend the analysis globaltag for accessing payloads
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# FEI list identifier
fei_identifier = 'feiHadronic'

# Define ROE masks for continuum suppression
track_mask = "[[dr < 2] and [abs(dz) < 4] and [pt > 0.2] and [thetaInCDCAcceptance==1]]"
ecl_mask = ("[[[[clusterReg==1] and [E>0.080]] or [[clusterReg==2] and [E > 0.03]] "
            "or [[clusterReg==3] and [E > 0.06]]] and [clusterNHits > 1.5] "
            "and [abs(clusterTiming) < 200] and [thetaInCDCAcceptance==1]]")
cleanMask = ("cleanMask", track_mask, ecl_mask)

# Apply FEI calibration cuts and build continuum suppression
for b in ['B+', 'B0']:
    # Apply cuts (matching FEI calibration)
    ma.applyCuts(f'{b}:{fei_identifier}', '[Mbc > 5.22] and [-0.15 < deltaE < 0.1]', path=my_path)

    # Build the Rest of Event
    ma.buildRestOfEvent(f'{b}:{fei_identifier}', path=my_path)
    ma.appendROEMasks(f'{b}:{fei_identifier}', [cleanMask], path=my_path)

    # Build continuum suppression (provides cosTBTO variable)
    ma.buildContinuumSuppression(f'{b}:{fei_identifier}', 'cleanMask', path=my_path)

    # Apply cosTBTO cut
    ma.applyCuts(f'{b}:{fei_identifier}', 'cosTBTO < 0.9', path=my_path)

    # TODO
    # BCS

# Build event shape variables (sphericity, thrust, etc.)
ma.buildEventShape(
    allMoments=False,
    cleoCones=False,
    jets=False,
    collisionAxis=False,
    harmonicMoments=True,
    foxWolfram=True,
    sphericity=True,
    thrust=True,
    path=my_path
)

# Define the particle lists to process
particle_lists = ['B+:feiHadronic', 'B0:feiHadronic']

# MC truth matching
for plist in particle_lists:
    ma.matchMCTruth(plist, path=my_path)

# Add D* veto reconstruction (pi0 list created internally)
modeSelector.addDstarVeto(particle_lists, path=my_path)

# Apply ModeSelector with local model files (for testing/development)
# Set debug=True to print feature values for comparison
modeSelector.modeSelector(
    particleLists=particle_lists,
    cat_model_path=args.cat_model,
    main_model_path=args.main_model,
    output_variable='BplusScore',
    debug=True,  # Enable debug output
    debug_max_events=3,  # Print first 3 events
    path=my_path
)

# TODO
# aliases

# Define output variables
output_variables = [
    # Basic kinematics
    'Mbc', 'deltaE', 'M',
    # Continuum suppression
    'cosTBTO',
    # FEI signal probability
    'extraInfo(SignalProbability)',
    'extraInfo(decayModeID)',
    # ModeSelector output (candidate-level)
    'extraInfo(BplusScore_eqSigProb)',
    # ModeSelector output (event-level)
    'eventExtraInfo(BplusScore)',
    'eventExtraInfo(BplusScore_catB0)',
    'eventExtraInfo(BplusScore_catBp)',
    'eventExtraInfo(BplusScore_catCont)',
    # MC truth matching
    'isSignal',
    # 'mostcommonBTagDeltaP',
    'mostcommonBTagIndex',
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
    filename=args.output + '_Bp.root',
    treename='Bp',
    path=my_path
)

# Write output ntuple for B0 candidates
ma.variablesToNtuple(
    'B0:feiHadronic',
    variables=output_variables,
    filename=args.output + '_B0.root',
    treename='B0',
    path=my_path
)

# Process events
b2.process(my_path)

# Print statistics
print(b2.statistics)
