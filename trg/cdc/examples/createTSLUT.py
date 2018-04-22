#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np

"""
Create a left/right LUT from a table of true left/right counts for each pattern.
(see generateTrueLRTable.py)

Condition for left/right:

left:    nBkg <= b * nTotal and nLeft > p * nMC + 3 * sqrt(p * (1 - p) * nMC)
right:   nBkg <= b * nTotal and nRight > p * nMC + 3 * sqrt(p * (1 - p) * nMC)
unknown: otherwise
"""

# load table with true left/right
# shape: [[nTrueRight, nTrueLeft, nTrueBkg], ...]
innerTrueLRTable = np.loadtxt('innerTrueLRTable_Bkg1.0_1.dat')
outerTrueLRTable = np.loadtxt('outerTrueLRTable_Bkg1.0_1.dat')
for i in range(2, 5):
    innerTrueLRTable += np.loadtxt('innerTrueLRTable_Bkg1.0_%d.dat' % i)
    outerTrueLRTable += np.loadtxt('outerTrueLRTable_Bkg1.0_%d.dat' % i)

# define thresholds for left/right
b = 0.8
p = 0.7

# filenames for the new LUTs
innerLUTFilename = "innerLUT_Bkg_p%.2f_b%.2f.coe" % (p, b)
outerLUTFilename = "outerLUT_Bkg_p%.2f_b%.2f.coe" % (p, b)


def isValidInnerPattern(pattern):
    masks = [(1 << 1),
             (1 << 2) + (1 << 3),
             (1 << 4) + (1 << 5) + (1 << 6),
             (1 << 7) + (1 << 8) + (1 << 9) + (1 << 10),
             (1 << 11) + (1 << 12) + (1 << 13) + (1 << 14) + (1 << 15)]
    nLayers = 0
    for i in range(5):
        if pattern & masks[i]:
            nLayers += 1
    return (nLayers >= 4)


def isValidOuterPattern(pattern):
    masks = [(1 << 1) + (1 << 2) + (1 << 3),
             (1 << 4) + (1 << 5),
             (1 << 6),
             (1 << 7) + (1 << 8),
             (1 << 9) + (1 << 10) + (1 << 11)]
    nLayers = 0
    for i in range(5):
        if pattern & masks[i]:
            nLayers += 1
    return (nLayers >= 4)


def createLUT(TrueLRTable, inner):
    LUT = np.zeros(len(TrueLRTable))
    # loop over patterns and check fraction of correct left/right
    for pattern, trueLR in enumerate(TrueLRTable):
        # check if pattern is valid
        if inner:
            if not isValidInnerPattern(pattern):
                continue
        else:
            if not isValidOuterPattern(pattern):
                continue
        if trueLR[2] > b * np.sum(trueLR):
            LUT[pattern] = 3
            continue
        threshold = p * np.sum(trueLR[:2]) + 3 * np.sqrt(p * (1 - p) * np.sum(trueLR[:2]))
        if trueLR[0] > threshold:
            LUT[pattern] = 1
        elif trueLR[1] > threshold:
            LUT[pattern] = 2
        else:
            LUT[pattern] = 3
    return LUT


innerLUT = createLUT(innerTrueLRTable, inner=True)
outerLUT = createLUT(outerTrueLRTable, inner=False)

# save the resulting LUTs
innerLUTFile = open(innerLUTFilename, 'w')
innerLUTFile.write("memory_initialization_radix=10;\n")
innerLUTFile.write("memory_initialization_vector=\n")
innerLUTFile.write(",\n".join("%d" % i for i in innerLUT))
innerLUTFile.write(";\n\n")

outerLUTFile = open(outerLUTFilename, 'w')
outerLUTFile.write("memory_initialization_radix=10;\n")
outerLUTFile.write("memory_initialization_vector=\n")
outerLUTFile.write(",\n".join("%d" % i for i in outerLUT))
outerLUTFile.write(";\n\n")
