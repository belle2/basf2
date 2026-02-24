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
    'mostcommonBTagDeltaP',
    'percentageWrongParticlesBTag',
    'percentageMissingParticlesBTag',
    'extraInfo(looseMCMotherPDG)',
    'extraInfo(looseMCWrongDaughterN)',
    'mostcommonBTagPDG',  # PDG of the true tag B (for cross_deltaC1 vs cross_internal)
    # Generated B meson PDGs (indices 3 and 4 in MCParticle list = first and second B meson).
    # 'genParticle(3, varForMCGen(PDG))',
    # 'genParticle(4, varForMCGen(PDG))',
    # Event-level flags
    'isContinuumEvent',  # 1 for qqbar continuum, 0 for BB
    'eventRandom',       # For base fraction cut (e.g. eventRandom < 0.25 for 25% sample)
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


# Indices to REMOVE from the full 1643-feature array (all-zero columns + Mbc block 10).
# Complement of the 1065 selected features. Verified against offline training.
# train.py recomputes this dynamically and asserts equality - update here if it changes.
# Total features: 1643 - 578 = 1065 kept.
REMOVE_INPUTS = [
    14, 15, 18, 19, 66, 67, 68, 69, 92, 93, 94, 95, 98, 99, 118, 122, 134,
    135, 150, 151, 154, 155, 202, 203, 204, 205, 228, 229, 230, 231, 234, 235,
    254, 258, 270, 271, 286, 287, 290, 291, 338, 339, 340, 341, 364, 365, 366,
    367, 370, 371, 390, 394, 406, 407, 422, 423, 426, 427, 474, 475, 476, 477,
    500, 501, 502, 503, 506, 507, 526, 530, 542, 543, 558, 559, 562, 563, 610,
    611, 612, 613, 636, 637, 638, 639, 642, 643, 662, 666, 678, 679, 694, 695,
    698, 699, 708, 709, 720, 721, 722, 723, 726, 727, 728, 729, 730, 731, 732,
    733, 734, 735, 736, 737, 738, 739, 740, 741, 746, 747, 748, 749, 750, 751,
    752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 764, 765, 766, 767, 768,
    769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783,
    784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798,
    799, 800, 801, 802, 803, 804, 805, 808, 809, 810, 811, 814, 815, 830, 831,
    834, 835, 838, 839, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852,
    853, 854, 855, 856, 857, 858, 859, 866, 867, 868, 869, 870, 871, 872, 873,
    874, 875, 876, 877, 880, 881, 882, 883, 884, 885, 886, 887, 908, 909, 910,
    911, 914, 915, 916, 917, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937,
    938, 939, 940, 941, 950, 951, 966, 967, 970, 971, 980, 981, 992, 993, 994,
    995, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009,
    1010, 1011, 1012, 1013, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025,
    1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1036, 1037, 1038, 1039,
    1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051,
    1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063,
    1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071, 1072, 1073, 1074, 1075,
    1076, 1077, 1080, 1081, 1082, 1083, 1086, 1087, 1102, 1103, 1106, 1107,
    1110, 1111, 1114, 1115, 1116, 1117, 1118, 1119, 1120, 1121, 1122, 1123,
    1124, 1125, 1126, 1127, 1128, 1129, 1130, 1131, 1138, 1139, 1140, 1141,
    1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1152, 1153, 1154, 1155,
    1156, 1157, 1158, 1159, 1180, 1181, 1182, 1183, 1186, 1187, 1188, 1189,
    1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1210, 1211,
    1212, 1213, 1222, 1223, 1238, 1239, 1242, 1243, 1290, 1291, 1292, 1293,
    1316, 1317, 1318, 1319, 1322, 1323, 1342, 1346, 1358, 1359,
    # Mbc block (feature block 10, indices 1360-1495):
    1360, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371,
    1372, 1373, 1374, 1375, 1376, 1377, 1378, 1379, 1380, 1381, 1382, 1383,
    1384, 1385, 1386, 1387, 1388, 1389, 1390, 1391, 1392, 1393, 1394, 1395,
    1396, 1397, 1398, 1399, 1400, 1401, 1402, 1403, 1404, 1405, 1406, 1407,
    1408, 1409, 1410, 1411, 1412, 1413, 1414, 1415, 1416, 1417, 1418, 1419,
    1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428, 1429, 1430, 1431,
    1432, 1433, 1434, 1435, 1436, 1437, 1438, 1439, 1440, 1441, 1442, 1443,
    1444, 1445, 1446, 1447, 1448, 1449, 1450, 1451, 1452, 1453, 1454, 1455,
    1456, 1457, 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467,
    1468, 1469, 1470, 1471, 1472, 1473, 1474, 1475, 1476, 1477, 1478, 1479,
    1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487, 1488, 1489, 1490, 1491,
    1492, 1493, 1494, 1495,
    1510, 1511, 1514, 1515, 1562, 1563, 1564, 1565, 1588, 1589, 1590, 1591,
    1594, 1595, 1614, 1618, 1630, 1631,
]  # 578 indices removed -> 1065 features kept
