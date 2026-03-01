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

import numpy as _np

# Number of FEI decay modes per B type (dmID range)
N_BP_MODES = 36  # B+/B- decay modes (dmID 0-35)
N_B0_MODES = 32  # B0/anti-B0 decay modes (dmID 0-31)

# Total input_id slots: B+ sector (0 to 2*N_BP_MODES-1) + B0 sector (2*N_BP_MODES to N_INPUT_IDS-1)
# Encoding: offset + dmID * 2 + is_particle (1 if PDG > 0 else 0)
N_INPUT_IDS = N_BP_MODES * 2 + N_B0_MODES * 2  # = 136

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
    'isSignal',
    'PDG',
    'extraInfo(decayModeID)',
    'Mbc',
    'extraInfo(SignalProbability)',
    'mostcommonBTagDeltaP',
    'mostcommonBTagPDG',
    'isContinuumEvent',
    'eventRandom',
]

# FEI calibration factors for sampling
# Maps decayModeID to weight for B+ and B0
# From FEI_cal_factors tables with sigProb > 0.01 threshold
FEI_CALIB_BP = {
    0: 1.118901, 1: 0.870929, 3: 1.323061, 4: 1.206390,
    15: 1.082857, 16: 1.213346, 18: 1.183716, 19: 1.757377,
    23: 1.188785, 24: 1.441938, 30: 0.225780,
}
FEI_CALIB_BP_REST = 0.848726

FEI_CALIB_B0 = {
    0: 1.181143, 1: 1.149421, 3: 1.291726, 4: 1.303912,
    5: 0.919011, 15: 1.092668, 16: 1.292981, 18: 1.269102,
    19: 2.001476, 26: 0.472930,
}
FEI_CALIB_B0_REST = 1.143776

# Normalisation reference for FEI calibration weights: 90th percentile of all calibration values
# across B+ and B0. Weights are divided by this before multiplying by fraction to give sample_prob.
# Events where sample_prob exceeds 1.0 after this are capped and reported as a warning.
# Using a percentile rather than the maximum prevents outlier modes from compressing the
# sampling of all other events.
_all_calib = (
    list(FEI_CALIB_BP.values()) + [FEI_CALIB_BP_REST] +
    list(FEI_CALIB_B0.values()) + [FEI_CALIB_B0_REST]
)
CALIB_WEIGHT_CAP = float(_np.percentile(_all_calib, 90, method='closest_observation'))
del _np, _all_calib


# Indices to KEEP from the full 1644-feature array (all-zero columns removed,
# Mbc block 10 excluded, treefitter chiProb columns excluded).
# train.py recomputes this dynamically and asserts equality -- update here
# if it changes.
HAS_INPUTS = [
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    18, 21, 23, 24, 25, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 63, 65, 71, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 152, 153, 154, 157, 159, 160, 161, 163, 164, 165,
    166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
    183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 199, 201,
    207, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 288,
    289, 290, 293, 295, 296, 297, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
    310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326,
    327, 328, 329, 330, 331, 332, 333, 335, 337, 343, 408, 409, 410, 411, 412, 413, 414,
    415, 416, 417, 418, 419, 420, 421, 422, 424, 425, 426, 429, 431, 432, 433, 435, 436,
    437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453,
    454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467, 468, 469, 471,
    473, 479, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558,
    560, 561, 562, 565, 567, 568, 569, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580,
    581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597,
    598, 599, 600, 601, 602, 603, 604, 605, 607, 609, 615, 681, 683, 685, 687, 689, 690,
    691, 693, 697, 701, 703, 705, 707, 711, 713, 715, 717, 719, 725, 734, 740, 743, 745,
    816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 832, 833,
    834, 837, 840, 841, 846, 848, 850, 852, 854, 861, 863, 865, 870, 872, 874, 876, 879,
    975, 979, 983, 985, 987, 989, 991, 1017, 1104, 1106, 1118, 1120, 1122, 1124, 1126, 1146, 1224,
    1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1238, 1240, 1241, 1242,
    1245, 1247, 1248, 1249, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258, 1259, 1260, 1261, 1262, 1263,
    1264, 1265, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1273, 1274, 1275, 1276, 1277, 1278, 1279, 1280,
    1281, 1282, 1283, 1284, 1285, 1287, 1289, 1295, 1496, 1497, 1498, 1499, 1500, 1501, 1502, 1503, 1504,
    1505, 1506, 1507, 1508, 1509, 1510, 1512, 1513, 1514, 1517, 1519, 1520, 1521, 1523, 1524, 1525, 1526,
    1527, 1528, 1529, 1530, 1531, 1532, 1533, 1534, 1535, 1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543,
    1544, 1545, 1546, 1547, 1548, 1549, 1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1559, 1561, 1567,
    1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639, 1640, 1641, 1642, 1643,
]  # 505 indices kept (1644 - 1139 removed)
