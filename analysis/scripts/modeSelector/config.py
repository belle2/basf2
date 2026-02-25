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


# Indices to KEEP from the full 1643-feature array (complement of all-zero
# columns + Mbc block 10, 136 indices each).  1643 - 578 = 1065 kept.
# train.py recomputes this dynamically and asserts equality -- update here
# if it changes.
HAS_INPUTS = [
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 17, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
    38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 70, 71, 72, 73, 74, 75,
    76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 96,
    97, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
    116, 117, 119, 120, 121, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 136,
    137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 152, 153, 156, 157,
    158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174,
    175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 206, 207, 208, 209, 210, 211, 212,
    213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 232, 233,
    236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
    253, 255, 256, 257, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 272, 273,
    274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 288, 289, 292, 293, 294,
    295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311,
    312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328,
    329, 330, 331, 332, 333, 334, 335, 336, 337, 342, 343, 344, 345, 346, 347, 348, 349,
    350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 368, 369, 372,
    373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389,
    391, 392, 393, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 408, 409, 410,
    411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 424, 425, 428, 429, 430, 431,
    432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448,
    449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 465,
    466, 467, 468, 469, 470, 471, 472, 473, 478, 479, 480, 481, 482, 483, 484, 485, 486,
    487, 488, 489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 504, 505, 508, 509,
    510, 511, 512, 513, 514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 527,
    528, 529, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541, 544, 545, 546, 547,
    548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 560, 561, 564, 565, 566, 567, 568,
    569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585,
    586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602,
    603, 604, 605, 606, 607, 608, 609, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623,
    624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 640, 641, 644, 645, 646,
    647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 663, 664,
    665, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 680, 681, 682, 683, 684,
    685, 686, 687, 688, 689, 690, 691, 692, 693, 696, 697, 700, 701, 702, 703, 704, 705,
    706, 707, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 724, 725, 742, 743, 744,
    745, 762, 763, 806, 807, 812, 813, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825,
    826, 827, 828, 829, 832, 833, 836, 837, 840, 841, 860, 861, 862, 863, 864, 865, 878,
    879, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 902, 903,
    904, 905, 906, 907, 912, 913, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 942,
    943, 944, 945, 946, 947, 948, 949, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961,
    962, 963, 964, 965, 968, 969, 972, 973, 974, 975, 976, 977, 978, 979, 982, 983, 984,
    985, 986, 987, 988, 989, 990, 991, 996, 997, 1014, 1015, 1016, 1017, 1034, 1035, 1078, 1079,
    1084, 1085, 1088, 1089, 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099, 1100, 1101, 1104,
    1105, 1108, 1109, 1112, 1113, 1132, 1133, 1134, 1135, 1136, 1137, 1150, 1151, 1160, 1161, 1162, 1163,
    1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1184,
    1185, 1190, 1191, 1192, 1193, 1194, 1195, 1196, 1197, 1198, 1199, 1214, 1215, 1216, 1217, 1218, 1219,
    1220, 1221, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1240,
    1241, 1244, 1245, 1246, 1247, 1248, 1249, 1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258, 1259,
    1260, 1261, 1262, 1263, 1264, 1265, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1273, 1274, 1275, 1276,
    1277, 1278, 1279, 1280, 1281, 1282, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1294, 1295, 1296, 1297,
    1298, 1299, 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311, 1312, 1313, 1314,
    1315, 1320, 1321, 1324, 1325, 1326, 1327, 1328, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337,
    1338, 1339, 1340, 1341, 1343, 1344, 1345, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356,
    1357, 1496, 1497, 1498, 1499, 1500, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508, 1509, 1512, 1513,
    1516, 1517, 1518, 1519, 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529, 1530, 1531, 1532,
    1533, 1534, 1535, 1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543, 1544, 1545, 1546, 1547, 1548, 1549,
    1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, 1559, 1560, 1561, 1566, 1567, 1568, 1569, 1570,
    1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, 1579, 1580, 1581, 1582, 1583, 1584, 1585, 1586, 1587,
    1592, 1593, 1596, 1597, 1598, 1599, 1600, 1601, 1602, 1603, 1604, 1605, 1606, 1607, 1608, 1609, 1610,
    1611, 1612, 1613, 1615, 1616, 1617, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, 1629,
    1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639, 1640, 1641, 1642,
]  # 1065 indices kept (1643 - 578 removed)
