#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Configuration for ModeSelector neural network.

This module contains the feature definitions, transformations, and constants
used by the ModeSelector module. Both inference and training must use these
definitions to ensure consistency.
"""

# Number of input_id values (dmID * 2 + is_charged)
N_INPUT_IDS = 136

# D* delta mass difference cut applied to features
DELTA_M_CUT = (-0.05, 0.05)

# Network output sizes
NUM_CAT_LABELS = 3   # B0, B+, continuum
NUM_MAIN_LABELS = 6  # bad_tag, is_target_neutral, cross_deltaC1, cross_internal, continuum, is_target_charged

# Feature block definitions: (name, basf2 variable, transform function)
# Block ordering matters - it defines the feature array layout.
#
#  0: sigProb            (1.0 + val * 100)
#  1: chiProb            (2.0 + val)
#  2: Bdaughter_sigProb  (1.0 + val * 10)
#  3: Bdaughter2_sigProb (1.0 + val * 10)
#  4: Bdaughter_chiProb  (2.0 + val)
#  5: Dst0_deltaMassDiff (1.0 + val * 20 + 0.5)
#  6: Dstp_deltaMassDiff (1.0 + val * 20 + 0.5)
#  7: Dst0_chiProb       (2.0 + val)
#  8: Dstp_chiProb       (2.0 + val)
#  9: deltaE             (1.0 + val * 5 + 0.75)
# 10: Mbc                (1.0 + (val - 5.23) * 20)  - EXCLUDED from training via has_inputs
# 11: cosTBTO            (1.0 + val)
#
FEATURE_BLOCKS = [
    ('sigProb', 'extraInfo(SignalProbability)', lambda x: 1.0 + x * 100),
    ('chiProb', 'chiProb', lambda x: 2.0 + x),
    ('Bdaughter_sigProb', 'daughter(0, extraInfo(SignalProbability))', lambda x: 1.0 + x * 10),
    ('Bdaughter2_sigProb', 'daughter(1, extraInfo(SignalProbability))', lambda x: 1.0 + x * 10),
    ('Bdaughter_chiProb', 'daughter(0, chiProb)', lambda x: 2.0 + x),
    ('Dst0_deltaMassDiff', 'extraInfo(Dst0_deltaMassDiff)', lambda x: 1.0 + x * 20 + 0.5),
    ('Dstp_deltaMassDiff', 'extraInfo(Dstp_deltaMassDiff)', lambda x: 1.0 + x * 20 + 0.5),
    ('Dst0_chiProb', 'extraInfo(Dst0_chiProb)', lambda x: 2.0 + x),
    ('Dstp_chiProb', 'extraInfo(Dstp_chiProb)', lambda x: 2.0 + x),
    ('deltaE', 'deltaE', lambda x: 1.0 + x * 5 + 0.75),
    ('Mbc', 'Mbc', lambda x: 1.0 + (x - 5.23) * 20),
    ('cosTBTO', 'cosTBTO', lambda x: 1.0 + x),
]

# Event-level features extracted from EventShapeContainer
EVENT_FEATURES = [
    'sphericity',
    'thrust',
    'thrustAxisCosTheta',
    'aplanarity',
    'foxWolframR2',
    'harmonicMomentThrust0',
    'harmonicMomentThrust1',
    'harmonicMomentThrust2',
]

# MC truth variables to evaluate on best candidates (for training labels)
TRAINING_MC_VARS = [
    'isSignalAcceptWrongFSPs',
    'isSignalAcceptMissing',
    'mcErrors',
    'PDG',
    'extraInfo(decayModeID)',
    'Mbc',
    'extraInfo(SignalProbability)',
    'mostcommonBTagIndex',
    # 'mostcommonBTagDeltaP',
    'percentageWrongParticlesBTag',
    'percentageMissingParticlesBTag',
    'extraInfo(looseMCMotherPDG)',
    'extraInfo(looseMCWrongDaughterN)',
    'isBBCrossfeed',
    # Generator B meson PDGs (for tag_is_gen_PDG computation)
    'genParticle(3, varForMCGen(PDG))',
    'genParticle(4, varForMCGen(PDG))',
]


def load_has_inputs(filepath):
    """
    Load the has_inputs list from a text file.

    The file should contain a Python list of integer indices.

    Parameters
    ----------
    filepath : str
        Path to the has_inputs file.

    Returns
    -------
    list of int
        Feature indices to select from the full feature array.
    """
    import ast
    with open(filepath, 'r') as f:
        return ast.literal_eval(f.read())
