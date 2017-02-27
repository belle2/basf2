#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to use the FSRCorrection
# module for the following decay:
#
#   J/psi
#    |
#    +-> e- e+
#        |  |
#        |  +-> e+ (gamma)
#        +-> e- (gamma)
#
# Note: The electrons in the corrected list have an
# attaced extraInfo 'fsrCorrected', indicating if
# the electron was corrected (1) or not (0).
#
# Contributors: Moritz Gelb (February 2017)
#
######################################################

from basf2 import *


# Bd_JpsiKL_ee Signal MC 7 file (each containing 2*10^6 generated events)
inputFile = "/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/" \
            "prod00000628/s00/e0000/4S/r00000/signal/sub00/mdst_000001_prod00000628_task00000001.root"
inputMdstList('default', inputFile)


# fill particleLists
fillParticleList('e+:uncorrected', 'eid > 0.2 and d0 < 2 and abs(z0) < 4')
fillParticleList('gamma:all', 'E < 1.0', False)

# fsr correction
correctFSR('e+:corrected', 'e+:uncorrected', 'gamma:all', 5.0, 1.0, False)

# reconstruct J/psi -> e+ e- decay
# keep only candidates with dM<0.11

# uncorrected
reconstructDecay('J/psi:uncorrected -> e+:uncorrected e-:uncorrected', 'dM<0.11')
applyCuts('J/psi:uncorrected', '3.07 < M < 3.11')

# corrected
reconstructDecay('J/psi:corrected -> e+:corrected e-:corrected', 'dM<0.11')
applyCuts('J/psi:corrected', '3.07 < M < 3.11')

# perform MC matching (MC truth asociation)
matchMCTruth('J/psi:uncorrcted')
matchMCTruth('J/psi:corrcted')

# write out ntuples
var0 = ['p',
        'E',
        'extraInfo(fsrCorrected)']
var1 = ['M',
        'p',
        'E',
        'daugher(0, p)',
        'daugher(1, p)',
        'daugher(0, extraInfo(fsrCorrected))',
        'daugher(1, extraInfo(fsrCorrected))']

variablesToNTuple('e+:uncorrected', var0, filename='e_uncorrected.root')
variablesToNTuple('e+:corrected', var0, filename='e_corrected.root')
variablesToNTuple('J/psi:uncorrected', var1, filename='Jpsi_uncorrected.root')
variablesToNTuple('J/psi:corrected', var1, filename='Jpsi_corrected.root')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
