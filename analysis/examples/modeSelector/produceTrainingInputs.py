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
# This script produces training inputs for the ModeSelector neural       #
# network. It runs the feature extraction (without NN inference) and     #
# saves the feature arrays + MC truth variables to numpy files.          #
#                                                                        #
# The output can be used directly for offline training.                  #
#                                                                        #
# Usage:                                                                 #
#   basf2 B2A921-ProduceTrainingInputs.py -- \                           #
#       --input <input_file.root> --output <output_prefix>               #
#                                                                        #
# Output files:                                                          #
#   <output_prefix>.npz          - event metadata + MC truth arrays      #
#   <output_prefix>_features.npz - sparse feature matrix (scipy)         #
#                                                                        #
##########################################################################

import argparse

import basf2 as b2
import modeSelector
import modularAnalysis as ma

# Parse arguments (basf2 strips its own args, remaining go to script)
parser = argparse.ArgumentParser()
parser.add_argument('--input', default='/home/pf/dataframes/MC16rd_skim/udst_000001_prod00051442_task230000001.root', nargs='+',
                    help='Input ROOT file(s) with FEI B meson candidates')
parser.add_argument('--output', default='modeSelector_training',
                    help='Output prefix for training files (default: modeSelector_training)')
parser.add_argument('--cont-fraction', type=float, default=0.25,
                    help='Continuum downsampling fraction relative to BB (default: 0.25)')
args = parser.parse_args()

# Set up logging
b2.set_log_level(b2.LogLevel.INFO)
b2.set_random_seed(1337)

# Create path
my_path = b2.create_path()

# Input files
ma.inputMdstList(filelist=args.input, path=my_path)

# Prepend the analysis globaltag
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Apply event fraction cuts using eventRandom.
# BB events: 40% base fraction. Continuum: additionally downsampled by cont_fraction.
BASE_FRACTION = 0.4
CONT_FRACTION = BASE_FRACTION * args.cont_fraction
b2.B2INFO(f"Applying event cuts: BB fraction={BASE_FRACTION}, continuum fraction={CONT_FRACTION}")
ma.applyEventCuts(
    f'[isContinuumEvent == 0 and eventRandom < {BASE_FRACTION}] or '
    f'[isContinuumEvent == 1 and eventRandom < {CONT_FRACTION}]',
    path=my_path
)

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
    ma.applyCuts(f'{b}:{fei_identifier}', '[Mbc > 5.23] and [-0.15 < deltaE < 0.1]', path=my_path)

    # Build the Rest of Event
    ma.buildRestOfEvent(f'{b}:{fei_identifier}', path=my_path)
    ma.appendROEMasks(f'{b}:{fei_identifier}', [cleanMask], path=my_path)

    # Build continuum suppression (provides cosTBTO variable)
    ma.buildContinuumSuppression(f'{b}:{fei_identifier}', 'cleanMask', path=my_path)

    # Apply cosTBTO cut
    ma.applyCuts(f'{b}:{fei_identifier}', 'cosTBTO < 0.9', path=my_path)

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
particle_lists = [f'B+:{fei_identifier}', f'B0:{fei_identifier}']

# MC truth matching (required for training labels)
for plist in particle_lists:
    ma.matchMCTruth(plist, path=my_path)

# Add D* veto reconstruction (pi0 list created internally)
modeSelector.addDstarVeto(particle_lists, path=my_path)

# Run ModeSelector in training mode (no NN models needed)
modeSelector.modeSelector(
    particleLists=particle_lists,
    training_mode=True,
    training_output=f'{args.output}.npz',
    path=my_path
)

# Process events
b2.process(my_path)

print(b2.statistics)
