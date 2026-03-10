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
parser.add_argument('--cat-model', default='onnx/modeSelector_cat.onnx',
                    help='Path to category ONNX model')
parser.add_argument('--main-model', default='onnx/modeSelector_main.onnx',
                    help='Path to main ONNX model')
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

# Prepend the analysis globaltag for accessing payloads
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# FEI list identifier
fei_identifier = 'feiHadronic'

# Define the particle lists to process
particle_lists = [f'B+:{fei_identifier}', f'B0:{fei_identifier}']

# MC truth matching
for plist in particle_lists:
    ma.matchMCTruth(plist, path=my_path)

# Keep an approximately complementary holdout band with 1 - eventRandom.
vm.addAlias('eventRandomComplement', 'formula(1 - eventRandom)')
modeSelector.addCalibratedEventRandomSampling(
    bp_list=f'B+:{fei_identifier}',
    b0_list=f'B0:{fei_identifier}',
    base_fraction=0.1,
    cont_fraction=1.0,
    random_variable='eventRandomComplement',
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

# Add D* veto reconstruction (pi0 list created internally)
modeSelector.addDstarVeto(particle_lists, path=my_path)

# Apply ModeSelector with local model files (for testing/development)
# Set debug=True to print feature values for comparison
modeSelector.modeSelector(
    bp_list=f'B+:{fei_identifier}',
    b0_list=f'B0:{fei_identifier}',
    cat_model_path=args.cat_model,
    main_model_path=args.main_model,
    output_variable='BplusScore',
    debug=False,  # Enable debug output
    debug_max_events=10,  # Print info for first 10 events, if debug is enabled
    path=my_path
)

# only AFTER running modeSelector
# Rank candidates in each list by two criteria stored for offline comparison:
# 1. sigProb_rank: pure sigProb ranking (all sigProb>0.001 candidates kept)
# 2. modeSelector_rank: BplusScore_eqSigProb in predicted sector, sigProb otherwise
vm.addAlias('BpModeSelectorRankVar', 'conditionalVariableSelector(BplusScore > 0, extraInfo(BplusScore_eqSigProb), sigProb)')
vm.addAlias('B0ModeSelectorRankVar', 'conditionalVariableSelector(BplusScore < 0, extraInfo(BplusScore_eqSigProb), sigProb)')

for b, eq_rank_var in zip(['B+', 'B0'], ['BpModeSelectorRankVar', 'B0ModeSelectorRankVar']):
    # rank by sigProb (pure), keep all candidates for offline comparison
    ma.rankByHighest(f'{b}:{fei_identifier}', 'sigProb',
                     numBest=0, outputVariable='sigProb_rank', path=my_path)
    # rank by eqSigProb in predicted sector, sigProb in non-predicted sector
    ma.rankByHighest(f'{b}:{fei_identifier}', eq_rank_var,
                     numBest=0, outputVariable='modeSelector_rank', path=my_path)

# for b, eq_rank_var in zip(['B+', 'B0'], ['BpModeSelectorRankVar', 'B0ModeSelectorRankVar']):
#     # this cut can remove some eqSigProb best candidates
#     # should be done after ranking for consistency between BplusScore and modeSelector_rank
#     ma.applyCuts(f'{b}:{fei_identifier}', 'sigProb > 0.001', path=my_path)

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

# isBestCandidate_eqSigProb: eqSigProb ranking; rank-1 in the predicted sector only
# could just define based on eqSigProb_rank, only considering predicted class?
vm.addAlias(
    'isBestCandidate_eqSigProb', 'conditionalVariableSelector( \
    [[BplusScore > 0] and [abs(PDG) == 521] and [modeSelector_rank == 1]] or \
    [[BplusScore < 0] and [abs(PDG) == 511] and [modeSelector_rank == 1]], \
    1, 0)'
)

vm.addAlias('BplusScore_cat', 'conditionalVariableSelector(BplusScore_catBp > BplusScore_catB0, 1, -1)')

# Keep at most 2 candidates per list: rank-1 by sigProb and rank-1 by eqSigProb.
# In the non-predicted sector eqSigProb falls back to sigProb, so only 1 is kept there.
# isBestCandidate_sigProb / isBestCandidate_eqSigProb stored in output for offline selection.
for b in ['B+', 'B0']:
    ma.applyCuts(f'{b}:{fei_identifier}',
                 '[sigProb_rank == 1] or [modeSelector_rank == 1]',
                 path=my_path)


# Create aliases for cleaner branch names in output ntuple
vm.addAlias('sigProb', 'extraInfo(SignalProbability)')
vm.addAlias('sigProb_rank', 'extraInfo(sigProb_rank)')
vm.addAlias('modeSelector_rank', 'extraInfo(modeSelector_rank)')
vm.addAlias('dmID', 'extraInfo(decayModeID)')
vu.create_aliases(
    ['BplusScore_eqSigProb', 'Dstp_deltaMassDiff', 'Dstp_chiProb', 'Dst0_deltaMassDiff', 'Dst0_chiProb'],
    wrapper='extraInfo({variable})'
)
vu.create_aliases(
    [
        'BplusScore',
        'BplusScore_catB0',
        'BplusScore_catBp',
        'BplusScore_catCont',
        'modeSelectorCalibWeight',
        'modeSelectorCalibInputId',
    ],
    wrapper='eventExtraInfo({variable})'
)

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
    'BplusScore_eqSigProb',
    # ModeSelector output (event-level)
    'BplusScore',
    'BplusScore_catB0',
    'BplusScore_catBp',
    'BplusScore_catCont',
    'modeSelectorCalibWeight',
    'modeSelectorCalibInputId',
    # +1 if pred charged, else -1
    'BplusScore_cat',
    # MC truth matching
    'isSignal',
    'PDG',
    'isContinuumEvent',
    'mostcommonBTagDeltaP',
    'mostcommonBTagPDG',
    # D* veto variables
    'Dstp_deltaMassDiff',
    'Dstp_chiProb',
    'Dst0_deltaMassDiff',
    'Dst0_chiProb',
    # ranking variables
    'sigProb_rank',
    'modeSelector_rank',
    'isBestCandidate_sigProb',
    'isBestCandidate_eqSigProb',
    # 'useCMSFrame(False)',
    # 'useCMSFrame(nCleanedTracks())',
    # 'nTrackFitResults(blah)',
]

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
