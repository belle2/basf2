#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np

"""
Compare a left/right LUT with a table of true left/right counts for each pattern.
(see generateTrueLRTable.py)
"""

# load LUTs to evaluate
innerLUT = np.loadtxt('trg/cdc/data/innerLUT_Bkg_p0.70_b0.80.coe',
                      skiprows=2, delimiter=',', usecols=[0], comments=';')
outerLUT = np.loadtxt('trg/cdc/data/outerLUT_Bkg_p0.70_b0.80.coe',
                      skiprows=2, delimiter=',', usecols=[0], comments=';')

# load table with true left/right
# shape: [[nTrueRight, nTrueLeft, nTrueBkg], ...]
innerTrueLRTable = np.loadtxt('innerTrueLRTable_Bkg1.0_5.dat')
outerTrueLRTable = np.loadtxt('outerTrueLRTable_Bkg1.0_5.dat')


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
        print("    %d correct, %d wrong, %d unknown"
              % (nCorrectMC, nWrongMC, nUnknownMC))
        print("    correct fraction", 100. * nCorrectMC / (nCorrectMC + nWrongMC),
              "+-", 100. * np.sqrt(nCorrectMC * nWrongMC / (nCorrectMC + nWrongMC) ** 3))
        print("    unknown fraction", 100. * nUnknownMC / nMC,
              "+-", 100. * np.sqrt(nUnknownMC * (nMC - nUnknownMC) / nMC ** 3))
    print("  %d TS with Bkg hit in priority wire" % nBkg)
    if nBkg > 0:
        print("    %d known, %d unknown"
              % (nKnownBkg, nUnknownBkg))
        print("    unknown fraction", 100. * nUnknownBkg / nBkg,
              "+-", 100. * np.sqrt(nUnknownBkg * (nBkg - nUnknownBkg) / nBkg ** 3))


innerCheck = check(innerLUT, innerTrueLRTable)
outerCheck = check(outerLUT, outerTrueLRTable)

print("\ninnermost super layer:")
printFractions(innerCheck)
print("\nouter super layers:")
printFractions(outerCheck)
print("\nall super layers:")
printFractions(np.array(innerCheck) + np.array(outerCheck))
