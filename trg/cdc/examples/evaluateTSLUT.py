#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np

"""
Compare a left/right LUT with a table of true left/right counts for each pattern.
(see generateTrueLRTable.py)
"""

# load LUTs to evaluate
innerLUT = np.loadtxt('innerLUT_p0.8_b0.0.coe',
                      skiprows=2, delimiter=',', usecols=[0], comments=';')
outerLUT = np.loadtxt('outerLUT_p0.8_b0.0.coe',
                      skiprows=2, delimiter=',', usecols=[0], comments=';')

# load table with true left/right
# shape: [[nTrueRight, nTrueLeft, nTrueBkg], ...]
innerTrueLRTable = np.loadtxt('innerTrueLRTable_Bkg0.0_2.dat')
outerTrueLRTable = np.loadtxt('outerTrueLRTable_Bkg0.0_2.dat')


def check(LUT, TrueLRTable):
    # loop over patterns and check fraction of correct left/right
    nCorrectMC = 0
    nWrongMC = 0
    nUnknownMC = 0
    nKnownBkg = 0
    nUnknownBkg = 0
    for pattern, trueLR in enumerate(TrueLRTable):
        LUTLR = LUT[pattern]
        if LUTLR < 3:
            nCorrectMC += trueLR[LUTLR - 1]
            nWrongMC += trueLR[2 - LUTLR]
            nKnownBkg += trueLR[2]
        else:
            nUnknownMC += trueLR[0] + trueLR[1]
            nUnknownBkg += trueLR[2]
    return nCorrectMC, nWrongMC, nUnknownMC, nKnownBkg, nUnknownBkg


def printFractions(checkResults):
    nCorrectMC, nWrongMC, nUnknownMC, nKnownBkg, nUnknownBkg = checkResults
    nMC = nCorrectMC + nWrongMC + nUnknownMC
    nBkg = nKnownBkg + nUnknownBkg
    print("  %d TS with MC hit in priority wire" % nMC)
    if nMC > 0:
        print("    %.3f correct, %.3f wrong, %.3f unknown"
              % (nCorrectMC / nMC, nWrongMC / nMC, nUnknownMC / nMC))
    print("  %d TS with Bkg hit in priority wire" % nBkg)
    if nBkg > 0:
        print("    %.3f known, %.3f unknown"
              % (nKnownBkg / nBkg, nUnknownBkg / nBkg))

innerCheck = check(innerLUT, innerTrueLRTable)
outerCheck = check(outerLUT, outerTrueLRTable)

print("\ninnermost super layer:")
printFractions(innerCheck)
print("\nouter super layers:")
printFractions(outerCheck)
print("\nall super layers:")
printFractions(np.array(innerCheck) + np.array(outerCheck))
