#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Generated-decay calibration weights for FEI B candidates.
"""

import basf2 as b2
from modeSelector import config
from ROOT import Belle2
from variables import variables as vm

DEFAULT_GEN_DMID_VARIABLE = 'genDecayModeID'
DEFAULT_WEIGHT_VARIABLE = 'genFEICalibWeight'
INVALID_DMID = -1
REST_DMID = 999

SIGNED_FSTATES = (511, 521, 211, 321, 411, 413, 421, 423, 431, 433, 2212, 4122)
UNSIGNED_FSTATES = (111, 310, 443)
DDSTAR_ABS_PDGS = frozenset((411, 413, 421, 423))
OTHER_ABS_PDGS = frozenset((11, 13, 2212, 2112, 130))
NO_ANCESTOR_ABS_PDGS = frozenset((211, 321, 411, 413, 421, 423, 431, 433, 2212, 4122, 111, 443))
NO_ANCESTOR_WO_DSP_ABS_PDGS = frozenset((211, 321, 411, 421, 423, 431, 433, 2212, 4122, 111, 443))
N_SIGNATURE_VALUES = len(SIGNED_FSTATES) * 2 + len(UNSIGNED_FSTATES) + 1
SIGNATURE_EXTRA_INFO_KEYS = (
    [f'n_neg_{pdg}' for pdg in SIGNED_FSTATES] +
    [f'n_pos_{pdg}' for pdg in SIGNED_FSTATES] +
    [f'n_pos_{pdg}' for pdg in UNSIGNED_FSTATES] +
    ['n_other']
)

MODE_SIGNATURE_MAP = {
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 0),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 0),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0): (-521, 1),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (521, 1),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0): (-521, 2),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0): (521, 2),
    (0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 3),
    (0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 3),
    (0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0): (-521, 4),
    (0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (521, 4),
    (0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 5),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 5),
    (0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0): (-521, 6),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (521, 6),
    (0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0): (-521, 7),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (521, 7),
    (0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0): (-521, 8),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (521, 8),
    (0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0): (-521, 9),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (521, 9),
    (0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 10),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 10),
    (0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 11),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 11),
    (0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 12),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (521, 12),
    (0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 13),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (521, 13),
    (0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 14),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0): (521, 14),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 15),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 15),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0): (-521, 16),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (521, 16),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0, 0): (-521, 17),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0): (521, 17),
    (0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 18),
    (0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 18),
    (0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0): (-521, 19),
    (0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (521, 19),
    (0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 20),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0): (521, 20),
    (0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 21),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0): (521, 21),
    (0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 22),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 22),
    (0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-521, 23),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (521, 23),
    (0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (-521, 24),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (521, 24),
    (0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0): (-521, 25),
    (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0): (521, 25),
    (0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0): (-521, 26),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0): (521, 26),
    (0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0): (-521, 27),
    (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0): (521, 27),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0): (-521, 28),
    (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0): (521, 28),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1): (-521, 29),
    (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1): (521, 29),
    (0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1): (-521, 30),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1): (521, 30),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 2): (-521, 31),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (521, 31),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 2): (-521, 32),
    (0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (521, 32),
    (0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (-521, 33),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (521, 33),
    (0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (-521, 34),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (521, 34),
    (0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1): (-521, 35),
    (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1): (521, 35),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 0),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (511, 0),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (-511, 1),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (511, 1),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0): (-511, 2),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0): (511, 2),
    (1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 3),
    (0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (511, 3),
    (1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (-511, 4),
    (0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (511, 4),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 5),
    (0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (511, 5),
    (1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 6),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (511, 6),
    (1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 7),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (511, 7),
    (1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 8),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0): (511, 8),
    (1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 9),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0): (511, 9),
    (1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (-511, 10),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (511, 10),
    (1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (-511, 11),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (511, 11),
    (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (-511, 12),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (511, 12),
    (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (-511, 13),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0): (511, 13),
    (1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 14),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0): (511, 14),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 15),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (511, 15),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (-511, 16),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (511, 16),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0): (-511, 17),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0): (511, 17),
    (1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 18),
    (0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (511, 18),
    (1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (-511, 19),
    (0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0): (511, 19),
    (1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 20),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0): (511, 20),
    (1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 21),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0): (511, 21),
    (1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0): (-511, 22),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0): (511, 22),
    (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0): (-511, 23),
    (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0): (511, 23),
    (1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0): (-511, 24),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0): (511, 24),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0): (-511, 25),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0): (511, 25),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1): (-511, 26),
    (0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1): (511, 26),
    (1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 2): (-511, 27),
    (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (511, 27),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (-511, 28),
    (0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (511, 28),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (-511, 29),
    (0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (511, 29),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 2): (-511, 30),
    (0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (511, 30),
    (1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 2): (-511, 31),
    (0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2): (511, 31),
}


def get_calibration_dm_id(pdg, exact_dm_id, rest_dmid=REST_DMID):
    """
    Collapse the exact generated FEI mode to the calibration granularity.
    """
    if abs(int(pdg)) not in (511, 521):
        return INVALID_DMID
    if exact_dm_id is None:
        return rest_dmid
    if exact_dm_id in config.get_fei_calibration_map(pdg):
        return int(exact_dm_id)
    return rest_dmid


def get_generated_calibration_weight(pdg, gen_dm_id, rest_dmid=REST_DMID):
    """
    Return the weight for a generated calibration dmID.
    """
    if abs(int(pdg)) not in (511, 521):
        return 1.0
    if gen_dm_id == INVALID_DMID:
        return 1.0
    if gen_dm_id == rest_dmid:
        return config.get_fei_calibration_rest(pdg)
    return config.get_fei_calibration_map(pdg).get(int(gen_dm_id), config.get_fei_calibration_rest(pdg))


def _abs_pdg(particle):
    return abs(int(particle.getPDG()))


def _is_primary_particle(particle):
    if hasattr(particle, 'isPrimaryParticle'):
        return bool(particle.isPrimaryParticle())
    if hasattr(particle, 'hasStatus') and hasattr(Belle2, 'MCParticle'):
        status_flag = getattr(Belle2.MCParticle, 'c_PrimaryParticle', None)
        if status_flag is not None:
            return bool(particle.hasStatus(status_flag))
    return True


def _belongs_to_root(particle, root_index):
    mother = particle.getMother()
    while mother:
        if int(mother.getArrayIndex()) == root_index:
            return True
        mother = mother.getMother()
    return False


def _ancestor_distance(particle, target_abs_pdg):
    distance = 1
    mother = particle.getMother()
    while mother:
        if _abs_pdg(mother) == target_abs_pdg:
            return distance
        mother = mother.getMother()
        distance += 1
    return 0


def _has_any_abs_ancestor(particle, abs_pdgs):
    mother = particle.getMother()
    while mother:
        if _abs_pdg(mother) in abs_pdgs:
            return True
        mother = mother.getMother()
    return False


def _count_descendants(descendants, predicate):
    return sum(1 for particle in descendants if predicate(particle))


def _build_mode_signature(root_particle, descendants):
    root_pdg = int(root_particle.getPDG())
    expand_dsp = False

    n_ddstar = _count_descendants(
        descendants,
        lambda particle: _abs_pdg(particle) in DDSTAR_ABS_PDGS and not _has_any_abs_ancestor(particle, DDSTAR_ABS_PDGS),
    )
    n_jpsi = _count_descendants(descendants, lambda particle: _abs_pdg(particle) == 443)
    n_ks = _count_descendants(
        descendants,
        lambda particle: _abs_pdg(particle) == 310 and not _has_any_abs_ancestor(particle, DDSTAR_ABS_PDGS),
    )
    n_p = _count_descendants(descendants, lambda particle: _abs_pdg(particle) == 2212)
    split_ks = (n_ddstar == 2 or n_jpsi == 1) and n_ks > 0
    if abs(root_pdg) == 521 and n_p == 0 and n_ks == 0:
        expand_dsp = True

    def count_signed(target_pdg, exclude_dsp=False):
        excluded = NO_ANCESTOR_WO_DSP_ABS_PDGS if exclude_dsp else NO_ANCESTOR_ABS_PDGS
        return _count_descendants(
            descendants,
            lambda particle: int(particle.getPDG()) == target_pdg and not _has_any_abs_ancestor(particle, excluded),
        )

    def count_signed_wo_ks(target_pdg, exclude_dsp=False):
        excluded = NO_ANCESTOR_WO_DSP_ABS_PDGS if exclude_dsp else NO_ANCESTOR_ABS_PDGS
        return _count_descendants(
            descendants,
            lambda particle: int(particle.getPDG()) == target_pdg
            and not _has_any_abs_ancestor(particle, excluded)
            and _ancestor_distance(particle, 310) == 0,
        )

    signature = []
    for pdg in SIGNED_FSTATES:
        if pdg in (511, 521):
            signature.append(1 if root_pdg == -pdg else 0)
            continue
        if pdg == 413:
            if expand_dsp:
                signature.append(0)
            elif split_ks:
                signature.append(count_signed_wo_ks(-pdg))
            else:
                signature.append(count_signed(-pdg))
            continue
        if expand_dsp:
            signature.append(count_signed(-pdg, exclude_dsp=True))
        elif split_ks:
            signature.append(count_signed_wo_ks(-pdg))
        else:
            signature.append(count_signed(-pdg))

    for pdg in SIGNED_FSTATES:
        if pdg in (511, 521):
            signature.append(1 if root_pdg == pdg else 0)
            continue
        if pdg == 413:
            if expand_dsp:
                signature.append(0)
            elif split_ks:
                signature.append(count_signed_wo_ks(pdg))
            else:
                signature.append(count_signed(pdg))
            continue
        if expand_dsp:
            signature.append(count_signed(pdg, exclude_dsp=True))
        elif split_ks:
            signature.append(count_signed_wo_ks(pdg))
        else:
            signature.append(count_signed(pdg))

    signature.append(count_signed(111, exclude_dsp=expand_dsp) if not split_ks else count_signed_wo_ks(111))
    if expand_dsp:
        signature.append(count_signed(310, exclude_dsp=True))
    elif split_ks:
        signature.append(count_signed_wo_ks(310))
    else:
        signature.append(0)
    signature.append(count_signed(443, exclude_dsp=expand_dsp) if not split_ks else count_signed_wo_ks(443))

    n_other = _count_descendants(
        descendants,
        lambda particle: (
            _abs_pdg(particle) in OTHER_ABS_PDGS
            and not _has_any_abs_ancestor(particle, NO_ANCESTOR_ABS_PDGS)
            and _ancestor_distance(particle, 310) == 0
        ) or (
            int(particle.getPDG()) == 22
            and particle.getEnergy() > 0.1
            and particle.getMother()
            and abs(int(particle.getMother().getPDG())) in (511, 521)
        ),
    )
    signature.append(n_other)
    return tuple(signature)


def _signature_to_dict(signature):
    return dict(zip(SIGNATURE_EXTRA_INFO_KEYS, signature))


def get_exact_dmid_from_signature(signature):
    """
    Return the exact FEI pdg and dmID for one generated-decay signature.
    """
    match = MODE_SIGNATURE_MAP.get(tuple(signature))
    if match is None:
        return None, None
    return match


def _match_generated_mode(root_particle, descendants, use_calibrated_dmids_only=True):
    signature = _build_mode_signature(root_particle, descendants)
    pdg, exact_dm_id = get_exact_dmid_from_signature(signature)
    if exact_dm_id is None:
        return {
            'pdg': int(root_particle.getPDG()),
            'exact_dm_id': None,
            'gen_dm_id': get_calibration_dm_id(root_particle.getPDG(), None),
            'weight': get_generated_calibration_weight(root_particle.getPDG(), REST_DMID),
            'signature': signature,
        }

    if use_calibrated_dmids_only:
        gen_dm_id = get_calibration_dm_id(pdg, exact_dm_id)
    else:
        gen_dm_id = int(exact_dm_id)
    return {
        'pdg': pdg,
        'exact_dm_id': exact_dm_id,
        'gen_dm_id': gen_dm_id,
        'weight': get_generated_calibration_weight(pdg, get_calibration_dm_id(pdg, exact_dm_id)),
        'signature': signature,
    }


def _build_event_truth_cache(particles, use_calibrated_dmids_only=True):
    cache = {}
    primary_particles = [particle for particle in particles if _is_primary_particle(particle)]
    roots = [particle for particle in primary_particles if _abs_pdg(particle) in (511, 521)]
    for root_particle in roots:
        root_index = int(root_particle.getArrayIndex())
        descendants = [
            particle for particle in primary_particles
            if _belongs_to_root(particle, root_index)
        ]
        cache[root_index] = _match_generated_mode(
            root_particle,
            descendants,
            use_calibrated_dmids_only=use_calibrated_dmids_only,
        )
    return cache


class GeneratedDecayWeightModule(b2.Module):
    """
    Assign generated-decay dmIDs and calibration weights to FEI candidates.
    """

    def __init__(
        self,
        bp_list,
        b0_list,
        gen_dmid_variable=DEFAULT_GEN_DMID_VARIABLE,
        weight_variable=DEFAULT_WEIGHT_VARIABLE,
        rest_dmid=REST_DMID,
        use_calibrated_dmids_only=True,
        store_btag_candidate_signature=False,
    ):
        super().__init__()
        self.bp_list = bp_list
        self.b0_list = b0_list
        self.gen_dmid_variable = gen_dmid_variable
        self.weight_variable = weight_variable
        self.rest_dmid = rest_dmid
        self.use_calibrated_dmids_only = use_calibrated_dmids_only
        self.store_btag_candidate_signature = store_btag_candidate_signature
        self._validated = False

    def initialize(self):
        self._validate_configuration()

    def _validate_configuration(self):
        if self._validated:
            return
        if not isinstance(self.bp_list, str) or not self.bp_list.startswith('B+:'):
            b2.B2FATAL(f"ModeSelector generated weights: invalid bp_list '{self.bp_list}'")
        if not isinstance(self.b0_list, str) or not self.b0_list.startswith('B0:'):
            b2.B2FATAL(f"ModeSelector generated weights: invalid b0_list '{self.b0_list}'")
        if not isinstance(self.gen_dmid_variable, str) or not self.gen_dmid_variable:
            b2.B2FATAL("ModeSelector generated weights: gen_dmid_variable must be a non-empty string")
        if not isinstance(self.weight_variable, str) or not self.weight_variable:
            b2.B2FATAL("ModeSelector generated weights: weight_variable must be a non-empty string")
        if int(self.rest_dmid) != REST_DMID:
            b2.B2FATAL(f"ModeSelector generated weights: rest_dmid must be {REST_DMID}")
        if not isinstance(self.use_calibrated_dmids_only, bool):
            b2.B2FATAL(
                "ModeSelector generated weights: use_calibrated_dmids_only must be bool"
            )
        if not isinstance(self.store_btag_candidate_signature, bool):
            b2.B2FATAL(
                "ModeSelector generated weights: store_btag_candidate_signature must be bool"
            )
        self._validated = True

    @staticmethod
    def _normalize_int(value):
        return None if value != value else int(value)

    @staticmethod
    def _set_extra_info(particle, key, value):
        if particle.hasExtraInfo(key):
            particle.setExtraInfo(key, value)
        else:
            particle.addExtraInfo(key, value)

    @staticmethod
    def _is_non_continuum_event(particle):
        is_cont = vm.evaluate('isContinuumEvent', particle)
        return int(is_cont) != 1

    @staticmethod
    def _report_missing_truth_annotation(list_name, particle, reason):
        b2.B2ERROR(
            "ModeSelector generated weights: non-continuum candidate missing generated "
            f"B truth annotation in {list_name} (array index {particle.getArrayIndex()}): {reason}"
        )

    def _annotate_list(self, list_name, truth_cache):
        plist = Belle2.PyStoreObj(list_name)
        if not plist.isValid():
            return

        particle_list = plist.obj()
        for i in range(plist.getListSize()):
            particle = particle_list.getParticle(i)
            btag_index = self._normalize_int(vm.evaluate('mostcommonBTagIndex', particle))
            tag_pdg = self._normalize_int(vm.evaluate('mostcommonBTagPDG', particle))
            is_non_continuum = self._is_non_continuum_event(particle)

            gen_dm_id = INVALID_DMID
            weight = config.FEI_CALIB_CONT
            match = None
            if btag_index is None:
                if is_non_continuum:
                    self._report_missing_truth_annotation(list_name, particle, 'mostcommonBTagIndex is NaN')
            elif tag_pdg is None:
                if is_non_continuum:
                    self._report_missing_truth_annotation(list_name, particle, 'mostcommonBTagPDG is NaN')
            elif abs(tag_pdg) not in (511, 521):
                if is_non_continuum:
                    self._report_missing_truth_annotation(
                        list_name,
                        particle,
                        f'mostcommonBTagPDG={tag_pdg} is not a B0 or B+ PDG'
                    )
            else:
                match = truth_cache.get(btag_index)
                if match is None:
                    if is_non_continuum:
                        self._report_missing_truth_annotation(
                            list_name,
                            particle,
                            f'mostcommonBTagIndex={btag_index} is not present in the event truth cache'
                        )
                else:
                    gen_dm_id = match['gen_dm_id']
                    weight = match['weight']

            self._set_extra_info(particle, self.gen_dmid_variable, float(gen_dm_id))
            self._set_extra_info(particle, self.weight_variable, float(weight))
            if self.store_btag_candidate_signature:
                if match is None:
                    signature_values = (0,) * N_SIGNATURE_VALUES
                else:
                    signature_values = match['signature']
                for key, value in _signature_to_dict(signature_values).items():
                    self._set_extra_info(particle, key, float(value))

    def event(self):
        bp_plist = Belle2.PyStoreObj(self.bp_list)
        b0_plist = Belle2.PyStoreObj(self.b0_list)
        bp_empty = not bp_plist.isValid() or bp_plist.getListSize() == 0
        b0_empty = not b0_plist.isValid() or b0_plist.getListSize() == 0
        if bp_empty and b0_empty:
            return

        mc_particles = Belle2.PyStoreArray('MCParticles')
        particles = [mc_particles[index] for index in range(mc_particles.getEntries())]
        truth_cache = _build_event_truth_cache(
            particles,
            use_calibrated_dmids_only=self.use_calibrated_dmids_only,
        )
        self._annotate_list(self.bp_list, truth_cache)
        self._annotate_list(self.b0_list, truth_cache)


def addGeneratedDecayWeights(
    bp_list,
    b0_list,
    gen_dmid_variable=DEFAULT_GEN_DMID_VARIABLE,
    weight_variable=DEFAULT_WEIGHT_VARIABLE,
    rest_dmid=REST_DMID,
    use_calibrated_dmids_only=True,
    store_btag_candidate_signature=False,
    path=None,
):
    """
    Add generated-decay calibration annotations to FEI candidate lists.
    """
    if path is None:
        b2.B2FATAL("ModeSelector generated weights: path is required")

    path.add_module(
        GeneratedDecayWeightModule(
            bp_list=bp_list,
            b0_list=b0_list,
            gen_dmid_variable=gen_dmid_variable,
            weight_variable=weight_variable,
            rest_dmid=rest_dmid,
            use_calibrated_dmids_only=use_calibrated_dmids_only,
            store_btag_candidate_signature=store_btag_candidate_signature,
        )
    )
