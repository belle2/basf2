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
#   <output>.pq  - parquet table with merged B+ and B0 candidates      #
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
parser.add_argument('--input', nargs='+',
                    default=['/home/pf/dataframes/MC16rd_skim/udst_000001_prod00051442_task230000001.root'],
                    help='Input ROOT file(s) with FEI B meson candidates')
parser.add_argument('--output', default='modeSelector_output',
                    help='Output parquet filename or stem (default: modeSelector_output)')
parser.add_argument('--cat-model', default=None,
                    help='Path to category ONNX model (omit to load from conditions DB)')
parser.add_argument('--main-model', default=None,
                    help='Path to main ONNX model (omit to load from conditions DB)')
parser.add_argument('--cat-payload-name', default='modeSelector_cat_model_v0',
                    help='Conditions DB payload name for the category model')
parser.add_argument('--main-payload-name', default='modeSelector_main_model_v0',
                    help='Conditions DB payload name for the main model')
parser.add_argument('--data', action='store_true',
                    help='Run in data mode: keep 10% of events with eventRandom and drop MC-only output variables')
args = parser.parse_args()

output_root, output_suffix = os.path.splitext(args.output)
if output_suffix.lower() in ['.pq', '.parquet']:
    final_output = args.output
elif output_suffix != '':
    final_output = output_root + '.pq'
else:
    final_output = args.output + '.pq'

# Set up logging
b2.set_log_level(b2.LogLevel.INFO)
b2.set_random_seed(1337)

# Create path
my_path = b2.create_path()

ma.inputMdstList(
    filelist=args.input,
    path=my_path
)

# b2.conditions.prepend_globaltag('user_feichtip_modeSelector')
b2.conditions.prepend_testing_payloads('localdb/database.txt')

# Prepend the analysis globaltag for accessing payloads
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

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

# Apply ModeSelector using local ONNX files when provided, otherwise load from
# the configured conditions DB globaltags or testing payloads.
# Set debug=True to print feature values for comparison.
modeSelector.modeSelector(
    bp_list=f'B+:{fei_identifier}',
    b0_list=f'B0:{fei_identifier}',
    cat_model_path=args.cat_model,
    main_model_path=args.main_model,
    payload_cat_model=args.cat_payload_name,
    payload_main_model=args.main_payload_name,
    output_variable='BplusScore',
    debug=False,  # Enable debug output
    debug_max_events=10,  # Print info for first 10 events, if debug is enabled
    path=my_path
)

# only AFTER running modeSelector
# Rank candidates by two criteria stored for offline comparison:
# 1. sigProb_rank: pure sigProb ranking (all sigProb>0.001 candidates kept)
# 2. modeSelector_rank: ModeSelector-based rank written by the module
for b in ['B+', 'B0']:
    # rank by sigProb (pure), keep all candidates for offline comparison
    ma.rankByHighest(f'{b}:{fei_identifier}', 'sigProb',
                     numBest=0, outputVariable='sigProb_rank', path=my_path)

# sigProb of rank-1 candidate in each list (for cross-sector comparison)
vm.addAlias('BpSigProb_rank1', 'ifNANgiveX(getVariableByRank(B+:feiHadronic, sigProb, sigProb, 1), -1)')
vm.addAlias('B0SigProb_rank1', 'ifNANgiveX(getVariableByRank(B0:feiHadronic, sigProb, sigProb, 1), -1)')

# isBestCandidate_sigProb: pure sigProb; rank-1 in the sector with the higher sigProb
vm.addAlias('sigProbOfBpGTB0', 'conditionalVariableSelector(BpSigProb_rank1 > B0SigProb_rank1, 1, 0)')
vm.addAlias('sigProbOfB0GTBp', 'conditionalVariableSelector(B0SigProb_rank1 > BpSigProb_rank1, 1, 0)')
vm.addAlias(
    'isBestCandidate_sigProb', 'conditionalVariableSelector( \
    [[sigProbOfBpGTB0 == 1] and [abs(PDG) == 521] and [sigProb_rank == 1]] or \
    [[sigProbOfB0GTBp == 1] and [abs(PDG) == 511] and [sigProb_rank == 1]], \
    1, 0)'
)

# Keep at most 2 candidates per list: rank-1 by sigProb and rank-1 by eqSigProb.
# isBestCandidate_sigProb stored in output for offline selection.
for b in ['B+', 'B0']:
    ma.applyCuts(f'{b}:{fei_identifier}', '[sigProb_rank == 1] or [modeSelector_rank == 1]', path=my_path)

if not args.data:
    modeSelector.addGeneratedDecayWeights(
        bp_list=f'B+:{fei_identifier}',
        b0_list=f'B0:{fei_identifier}',
        store_btag_candidate_signature=True,
        path=my_path
    )

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
    'modeSelector_feiCalibWeight',
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
    'modeSelector_feiCalibWeight',
    'PDG',
    # D* veto variables
    'Dstp_deltaMassDiff',
    'Dstp_chiProb',
    'Dst0_deltaMassDiff',
    'Dst0_chiProb',
    # ranking variables
    'sigProb_rank',
    'modeSelector_rank',
    'isBestCandidate_sigProb',
    'eventRandom',
]

if not args.data:
    output_variables.extend([
        'genDecayModeID',
        'genFEICalibWeight',
        'isSignal',
        'isContinuumEvent',
        'mostcommonBTagDeltaP',
        'mostcommonBTagPDG',
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
