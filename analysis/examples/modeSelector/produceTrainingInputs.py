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
# writes the feature arrays + MC truth variables to a ROOT ntuple file   #
# via basf2's own output modules, so the output is downloadable when     #
# this script is submitted with gbasf2 (see the examples README for a    #
# grid submission walkthrough).                                          #
#                                                                        #
# The output can be used directly for offline training.                  #
#                                                                        #
# Usage:                                                                 #
#   basf2 produceTrainingInputs.py -- \                                  #
#       --input <input_file.root> --output <output_prefix>               #
#                                                                        #
# Output file:                                                           #
#   <output_prefix>.root - ROOT file with two trees:                     #
#     events         - one row per event: 1644 raw features              #
#                       (modeSelector_feat_XXXX) + per-event truth/label  #
#                       scalars (modeSelector_tr_*)                      #
#     sig_candidates - one row per B+/B0 candidate; true signal           #
#                       candidates carry modeSelector_trainSigInputId     #
#                                                                        #
##########################################################################

import argparse

import basf2 as b2
import modeSelector
import modularAnalysis as ma
from modeSelector import config
from variables import variables as vm

# Parse arguments (basf2 strips its own args, remaining go to script)
parser = argparse.ArgumentParser()
parser.add_argument('--input', nargs='+', default=None,
                    help='Input ROOT file(s) with FEI B meson candidates')
parser.add_argument('--output', default='modeSelector_training',
                    help='Output prefix for training files (default: modeSelector_training)')
parser.add_argument('--cont-fraction', type=float, default=0.25,
                    help='Continuum keep fraction relative to the 30% BB band (default: 0.25)')
args = parser.parse_args()

# Resolved after parsing so that --input works without the validation file installed
if args.input is None:
    args.input = [b2.find_file('udst16_feiHadronic.root', 'validation')]

# Set up logging
b2.set_log_level(b2.LogLevel.INFO)
b2.set_random_seed(1337)

# Create path
my_path = b2.create_path()

# Input files
ma.inputMdstList(filelist=args.input, path=my_path)

# Prepend the analysis globaltag
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# FEI list identifier
fei_identifier = 'feiHadronic'

# Define the particle lists to process
particle_lists = [f'B+:{fei_identifier}', f'B0:{fei_identifier}']

# MC truth matching (required for training labels)
for plist in particle_lists:
    ma.matchMCTruth(plist, path=my_path)

# Apply simple eventRandom cuts on the raw FEI lists.
BASE_FRACTION = 0.3
ma.applyEventCuts(
    f'[[isContinuumEvent == 1] and [eventRandom < {BASE_FRACTION * args.cont_fraction}]] or '
    f'[[isContinuumEvent != 1] and [eventRandom < {BASE_FRACTION}]]',
    path=my_path
)

# Define ROE masks for continuum suppression
track_mask = "[[dr < 2] and [abs(dz) < 4] and [pt > 0.2] and [thetaInCDCAcceptance==1]]"
ecl_mask = ("[[[[clusterReg==1] and [E>0.080]] or [[clusterReg==2] and [E > 0.03]] "
            "or [[clusterReg==3] and [E > 0.06]]] and [clusterNHits > 1.5] "
            "and [abs(clusterTiming) < 200] and [thetaInCDCAcceptance==1]]")
cleanMask = ("cleanMask", track_mask, ecl_mask)

# Apply FEI calibration cuts and build continuum suppression on kept events.
for b in ['B+', 'B0']:
    ma.applyCuts(f'{b}:{fei_identifier}', '[Mbc > 5.23] and [-0.15 < deltaE < 0.1]', path=my_path)

    # Build the Rest of Event
    ma.buildRestOfEvent(f'{b}:{fei_identifier}', path=my_path)
    ma.appendROEMasks(f'{b}:{fei_identifier}', [cleanMask], path=my_path)

    # Build continuum suppression (provides cosTBTO variable)
    ma.buildContinuumSuppression(f'{b}:{fei_identifier}', 'cleanMask', path=my_path)

    # Apply cosTBTO cut
    ma.applyCuts(f'{b}:{fei_identifier}', 'cosTBTO < 0.9', path=my_path)

# Build event shape variables (sphericity, thrust, etc.) only for kept events.
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

modeSelector.addGeneratedDecayWeights(
    bp_list=f'B+:{fei_identifier}',
    b0_list=f'B0:{fei_identifier}',
    path=my_path
)

# Wrap B+ and B0 in a common Upsilon(4S) candidate and merge lists, so the
# per-candidate signal-truth ntuple below can dump both sectors with a single
# variablesToNtuple call (same pattern as applyModeSelector.py). ExtraInfo set by
# ModeSelector on the B+/B0 candidate is read back below via daughter(0, ...).
for b, b_str in zip(['B+', 'B0'], ['Bp', 'B0']):
    ma.reconstructDecay(
        f'Upsilon(4S):{b_str} -> {b}:{fei_identifier}',
        '',
        allowChargeViolation=True,
        path=my_path
    )
ma.copyLists('Upsilon(4S):trainCandidates', ['Upsilon(4S):Bp', 'Upsilon(4S):B0'], path=my_path)

# Run ModeSelector in training mode (no NN models needed). Features and MC truth
# are exposed via EventExtraInfo/ExtraInfo for the variablesToNtuple calls below.
modeSelector.modeSelector(
    bp_list=f'B+:{fei_identifier}',
    b0_list=f'B0:{fei_identifier}',
    training_mode=True,
    path=my_path
)

output_filename = f'{args.output}.root'

# Event-level ntuple: raw features + per-event truth/label scalars, one row per event.
# Aliased to plain identifiers so the resulting branch names are predictable (no ROOT
# name-mangling of the eventExtraInfo(...)/extraInfo(...) meta-variable syntax).
n_raw_features = len(config.FEATURE_BLOCKS) * config.N_INPUT_IDS + len(config.EVENT_FEATURES) + 4
event_truth_fields = [
    'is_cont', 'gen_pdg', 'bp_gen_decay_mode_id', 'b0_gen_decay_mode_id',
    'bp_gen_fei_calib_weight', 'b0_gen_fei_calib_weight', 'bp_is_best',
    'best_sigprob', 'best_bp_sigprob_iid', 'best_b0_sigprob_iid',
    'bp_tag_is_gen', 'b0_tag_is_gen', 'fei_calib_weight',
    'best_bp_iid', 'best_bp_dp', 'best_b0_iid', 'best_b0_dp',
]
event_variables = []
for i in range(n_raw_features):
    alias = f'ms_feat_{i:04d}'
    vm.addAlias(alias, f'eventExtraInfo(modeSelector_feat_{i:04d})')
    event_variables.append(alias)
for name in event_truth_fields:
    alias = f'ms_tr_{name}'
    vm.addAlias(alias, f'eventExtraInfo(modeSelector_tr_{name})')
    event_variables.append(alias)

ma.variablesToNtuple(
    '', variables=event_variables, treename='events',
    filename=output_filename, path=my_path
)

# Candidate-level ntuple: one row per B+/B0 candidate. Rows with a set
# ms_sig_input_id are the deduplicated true-signal candidates.
sig_candidate_aliases = {
    'ms_sig_input_id': 'daughter(0, extraInfo(modeSelector_trainSigInputId))',
    'ms_sig_btag_index': 'daughter(0, mostcommonBTagIndex)',
    'ms_sig_delta_p': 'daughter(0, mostcommonBTagDeltaP)',
    'ms_sig_sigprob': 'daughter(0, extraInfo(SignalProbability))',
}
for alias, expression in sig_candidate_aliases.items():
    vm.addAlias(alias, expression)

ma.variablesToNtuple(
    'Upsilon(4S):trainCandidates',
    variables=list(sig_candidate_aliases.keys()),
    treename='sig_candidates',
    filename=output_filename, path=my_path
)

# Process events
b2.process(my_path)

print(b2.statistics)
