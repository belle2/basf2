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

This module contains the feature indices and transformations used during
training. The inference must match these exactly.
"""

# Feature indices that were non-zero during training
# These are the columns selected from the full sparse feature matrix
# Format: list of indices into the flattened feature array
#
# The feature array structure:
# - 11 feature blocks × 136 columns = 1496 base features
# - Plus 11 event-level features
# - Total: 1507 features before selection
#
# After selection with has_inputs, we get ~1004 features for the category network

# This list should be loaded from the training configuration or model metadata
# For now, we define a placeholder that should be replaced with actual values
HAS_INPUTS = None  # Will be loaded from model or config file

# Feature block definitions
FEATURE_BLOCKS = [
    # (name, variable, transform_func, default_value)
    ('sigProb', 'extraInfo(SignalProbability)', lambda x: 1.0 + x * 100, 0.0),
    ('chiProb', 'chiProb', lambda x: 2.0 + x, 0.0),
    ('Bdaughter_sigProb', 'daughter(0, extraInfo(SignalProbability))', lambda x: 1.0 + x * 10, 0.0),
    ('Bdaughter2_sigProb', 'daughter(1, extraInfo(SignalProbability))', lambda x: 1.0 + x * 10, 0.0),
    ('Bdaughter_chiProb', 'daughter(0, chiProb)', lambda x: 2.0 + x, 0.0),
    ('Dst0_deltaMassDiff', 'extraInfo(Dst0_deltaMassDiff)', lambda x: 1.0 + x * 20 + 0.5, 0.0),
    ('Dstp_deltaMassDiff', 'extraInfo(Dstp_deltaMassDiff)', lambda x: 1.0 + x * 20 + 0.5, 0.0),
    ('Dst0_chiProb', 'extraInfo(Dst0_chiProb)', lambda x: 2.0 + x, 0.0),
    ('Dstp_chiProb', 'extraInfo(Dstp_chiProb)', lambda x: 2.0 + x, 0.0),
    ('deltaE', 'deltaE', lambda x: 1.0 + x * 5 + 0.75, 0.0),
    # Note: Mbc (block 10) is excluded from training
    ('cosTBTO', 'cosThetaBetweenParticleAndNominalB', lambda x: 1.0 + x, 0.0),
]

# Event-level features
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

# Number of input_id values (dmID * 2 + is_charged)
N_INPUT_IDS = 136

# D* delta mass difference cut
DELTA_M_CUT = (-0.05, 0.05)

# Network configuration
NUM_CAT_LABELS = 3   # B0, B+, continuum
NUM_MAIN_LABELS = 6  # Various signal/background categories


def load_has_inputs_from_file(filepath):
    """
    Load the has_inputs list from a file.

    The file should contain a Python list of integers.
    """
    import ast
    with open(filepath, 'r') as f:
        content = f.read()
    return ast.literal_eval(content)


def save_has_inputs_to_file(has_inputs, filepath):
    """
    Save the has_inputs list to a file.
    """
    with open(filepath, 'w') as f:
        f.write(repr(has_inputs))
