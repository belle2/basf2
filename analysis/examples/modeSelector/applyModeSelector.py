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
# This script applies ModeSelector to FEI B meson candidates and writes  #
# candidate-level and event-level output variables to parquet.           #
#                                                                        #
# Usage:                                                                 #
#   basf2 applyModeSelector.py -- [options]                              #
#                                                                        #
# Output files:                                                          #
#   <output>.pq  - parquet table with merged B+ and B0 candidates        #
#                                                                        #
##########################################################################

import argparse
import os

import basf2 as b2
import modeSelector
import modularAnalysis as ma
import variables.utils as vu
from b2pandas_utils import VariablesToTable
from variables import variables as vm

parser = argparse.ArgumentParser()
parser.add_argument('--input', nargs='+', default=None,
                    help='Input ROOT file(s) with FEI B meson candidates')
parser.add_argument('--output', default='modeSelector_output',
                    help='Output parquet filename or stem (default: modeSelector_output)')
parser.add_argument('--cat-model', default=None,
                    help='Path to category MVA ONNX weightfile (omit to use payloads)')
parser.add_argument('--main-model', default=None,
                    help='Path to main MVA ONNX weightfile (omit to use payloads)')
parser.add_argument('--cat-payload-name', default=None,
                    help='Conditions DB payload name for the category model (default: derived from the contract version)')
parser.add_argument('--main-payload-name', default=None,
                    help='Conditions DB payload name for the main model (default: derived from the contract version)')
parser.add_argument('--globaltag', default=None,
                    help='Additional globaltag holding the ModeSelector payloads, prepended to the analysis globaltag')
parser.add_argument('--data', action='store_true',
                    help='Run in data mode: keep 10% of events with eventRandom and drop MC-only output variables')
args = parser.parse_args()

# Resolved after parsing so that --input works without the validation file installed
if args.input is None:
    args.input = [b2.find_file('udst16_feiHadronic.root', 'validation')]

output_root, output_suffix = os.path.splitext(args.output)
if output_suffix.lower() in ['.pq', '.parquet']:
    final_output = args.output
elif output_suffix != '':
    final_output = output_root + '.pq'
else:
    final_output = args.output + '.pq'

# Set up logging
b2.set_random_seed(1337)

# Create path
my_path = b2.create_path()

ma.inputMdstList(
    filelist=args.input,
    path=my_path
)

# analysis globaltag for accessing payloads
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# The ModeSelector payloads are not in the analysis globaltag yet, so the
# globaltag holding them has to be given explicitly when loading from the
# conditions database.
if args.globaltag:
    b2.conditions.prepend_globaltag(args.globaltag)

# FEI list identifier
fei_identifier = 'feiHadronic'

# Define the particle lists to process
particle_lists = [f'B+:{fei_identifier}', f'B0:{fei_identifier}']

if args.data:
    ma.applyEventCuts('eventRandom < 0.1', path=my_path)
else:
    # MC truth matching
    for plist in particle_lists:
        ma.matchMCTruth(plist, path=my_path)

    # Keep a complementary high-band holdout sample for smaller MC output.
    ma.applyEventCuts('eventRandom > 0.95', path=my_path)

# Define ROE masks for continuum suppression
track_mask = "[[dr < 2] and [abs(dz) < 4] and [pt > 0.2] and [thetaInCDCAcceptance==1]]"
ecl_mask = ("[[[[clusterReg==1] and [E>0.080]] or [[clusterReg==2] and [E > 0.03]] "
            "or [[clusterReg==3] and [E > 0.06]]] and [clusterNHits > 1.5] "
            "and [abs(clusterTiming) < 200] and [thetaInCDCAcceptance==1]]")
cleanMask = ("cleanMask", track_mask, ecl_mask)

# Apply FEI preselection and build continuum suppression on kept events.
for b in ['B+', 'B0']:
    ma.applyCuts(f'{b}:{fei_identifier}', '[Mbc > 5.23] and [-0.15 < deltaE < 0.1]', path=my_path)

    # Build the Rest of Event
    ma.buildRestOfEvent(f'{b}:{fei_identifier}', path=my_path)
    ma.appendROEMasks(f'{b}:{fei_identifier}', [cleanMask], path=my_path)

    # Build continuum suppression (provides cosTBTO variable)
    ma.buildContinuumSuppression(f'{b}:{fei_identifier}', 'cleanMask', path=my_path)

    # Apply cosTBTO cut
    ma.applyCuts(f'{b}:{fei_identifier}', 'cosTBTO < 0.9', path=my_path)

    # rank by sigProb, do not cut yet
    ma.rankByHighest(f'{b}:{fei_identifier}', 'sigProb', outputVariable='sigProb_rank', path=my_path)

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

# Set debug=True to print feature values for comparison.
modeSelector.modeSelector(
    bp_list=f'B+:{fei_identifier}',
    b0_list=f'B0:{fei_identifier}',
    cat_model_path=args.cat_model,
    main_model_path=args.main_model,
    payload_cat_model=args.cat_payload_name,
    payload_main_model=args.main_payload_name,
    output_variable='BplusScore',
    store_fei_calib_weight=not args.data,
    debug=False,  # Enable debug output
    debug_max_events=10,  # Print info for first 10 events, if debug is enabled
    path=my_path
)

# Candidates are ranked by two criteria stored for offline comparison:
# 1. sigProb_rank: pure sigProb ranking
# 2. modeSelector_rank: ModeSelector-based rank written by the module

# cut only AFTER running modeSelector
# Keep at most 2 candidates per list: rank-1 by sigProb and rank-1 by eqSigProb.
# isBestCandidate_sigProb stored in output for offline selection.
for b in ['B+', 'B0']:
    ma.applyCuts(f'{b}:{fei_identifier}', '[sigProb_rank == 1] or [modeSelector_rank == 1]', path=my_path)

if not args.data:
    modeSelector.addGeneratedDecayWeights(
        bp_list=f'B+:{fei_identifier}',
        b0_list=f'B0:{fei_identifier}',
        # store_btag_candidate_signature=True,
        path=my_path
    )

ma.fillParticleList(decayString="pi+:test", cut='', path=my_path)

# global rank
vm.addAlias('sigProbRank_global', f'sigProbRank(B+:{fei_identifier}, B0:{fei_identifier})')
# isBestCandidate_sigProb: rank-1 sigProb candidate in the sector with the higher rank-1 sigProb
vm.addAlias('isBestCandidate_sigProb', 'conditionalVariableSelector(sigProbRank_global == 1, 1, 0)')

# Create aliases for cleaner branch names in output ntuple
vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
vm.addAlias('dmID', 'extraInfo(decayModeID)')

candidate_variables = [
    'sigProb_rank',
    'modeSelector_rank',
    'modeSelector_eqSigProb',
    'Dstp_deltaMassDiff',
    'Dstp_chiProb',
    'Dst0_deltaMassDiff',
    'Dst0_chiProb',
    'genDecayModeID',
    'genFEICalibWeight',
]
vu.create_aliases(candidate_variables, wrapper='extraInfo({variable})')

event_output_variables = [
    'BplusScore',
    'modeSelector_catB0',
    'modeSelector_catBp',
    'modeSelector_catCont',
]
vu.create_aliases(event_output_variables, wrapper='eventExtraInfo({variable})')

# Define output variables
output_variables = [
    # Basic kinematics
    'Mbc', 'deltaE', 'M',
    # Continuum suppression
    'cosTBTO',
    # FEI signal probability
    'sigProb',
    'dmID',
    # ModeSelector output (candidate-level)
    'modeSelector_eqSigProb',
    # ModeSelector output (event-level)
    'BplusScore',
    'modeSelector_catB0',
    'modeSelector_catBp',
    'modeSelector_catCont',
    'PDG',
    # D* veto variables
    'Dstp_deltaMassDiff',
    'Dstp_chiProb',
    'Dst0_deltaMassDiff',
    'Dst0_chiProb',
    # ranking variables
    'sigProb_rank',
    'modeSelector_rank',
    'sigProbRank_global',
    'isBestCandidate_sigProb',
    'eventRandom',
]

if not args.data:
    vm.addAlias('modeSelector_feiCalibWeight', 'eventExtraInfo(modeSelector_feiCalibWeight)')
    output_variables.extend([
        'genDecayModeID',
        'genFEICalibWeight',
        'isSignal',
        'isContinuumEvent',
        'mostcommonBTagDeltaP',
        'mostcommonBTagPDG',
        'modeSelector_feiCalibWeight',
    ])

# Wrap B+ and B0 in a common Upsilon(4S) candidate and merge lists
for b, b_str in zip(['B+', 'B0'], ['Bp', 'B0']):
    ma.reconstructDecay(
        f'Upsilon(4S):{b_str} -> {b}:{fei_identifier}',
        '',
        allowChargeViolation=True,
        path=my_path
    )

ma.copyLists(
    outputListName='Upsilon(4S):all',
    inputListNames=['Upsilon(4S):Bp', 'Upsilon(4S):B0'],
    path=my_path
)

# Save daughter(0, ...) variables with B_ prefix in output columns
merged_output_variables = vu.create_daughter_aliases(
    output_variables,
    [0],
    prefix='B',
    include_indices=False
)

v2t = VariablesToTable(
    'Upsilon(4S):all',
    variables=merged_output_variables,
    filename=final_output,
    event_buffer_size=500_000,
)
my_path.add_module(v2t)

# Process events
b2.process(my_path)

# Print statistics
print(b2.statistics)
