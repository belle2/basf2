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
# Notes:
#
# 1) The electrons in the corrected list have an
#    attaced extraInfo 'fsrCorrected', indicating if
#    the electron was corrected (1) or not (0).
# 2) It's recommended to use looseMCMatching.
#
# Contributors: Moritz Gelb (February 2017)
#
######################################################

from basf2 import *
from modularAnalysis import *

# set the log level
set_log_level(LogLevel.WARNING)

# Bd_JpsiKL_ee Signal MC 7 file
inputFile = "/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/" \
            "prod00000628/s00/e0000/4S/r00000/signal/sub00/mdst_000001_prod00000628_task00000001.root"
inputMdstList('default', inputFile)


# fill particleLists
fillParticleList('e+:uncorrected', 'electronID > 0.2 and d0 < 2 and abs(z0) < 4')
fillParticleList('gamma:all', 'E < 1.0', False)

# loose mc matching (recommended)
looseMCTruth('e+:uncorrected')
looseMCTruth('gamma:all')

# fsr correction
correctFSR('e+:corrected', 'e+:uncorrected', 'gamma:all', 5.0, 1.0, False)
looseMCTruth('e+:corrected')

# uncorrected
reconstructDecay('J/psi:uncorrected -> e+:uncorrected e-:uncorrected', '')
reconstructDecay('J/psi:corrected -> e+:corrected e-:corrected', '')

# vertex fit
# vertexRave('J/psi:uncorrected', 0.0)
# vertexRave('J/psi:corrected', 0.0)

# loose MC matching
looseMCTruth('J/psi:uncorrected')
looseMCTruth('J/psi:corrected')

# get all MC particles
fillParticleListFromMC('J/psi:MC', '', False, False)

# write out ntuples
var0 = ['p',
        'px',
        'py',
        'pz',
        'x',
        'y',
        'z',
        'PDG',
        'mcPDG',
        'E',
        'mcPX',
        'mcPY',
        'mcPZ',
        'mcE',
        'pxErr',
        'pyErr',
        'pzErr',
        'pErr',
        'isSignal',
        'mcErrors',
        'extraInfo(fsrCorrected)',
        'extraInfo(looseMCMotherPDG)',
        'extraInfo(looseMCMotherIndex)',
        'extraInfo(looseMCWrongDaughterN)',
        'extraInfo(looseMCWrongDaughterPDG)',
        'extraInfo(looseMCWrongDaughterBiB)',
        ]
var1 = ['M',
        'p',
        'E',
        'x', 'y', 'z',
        'isSignal',
        'mcErrors',
        'daughter(0, p)',
        'daughter(1, p)',
        'daughter(0, extraInfo(fsrCorrected))',
        'daughter(1, extraInfo(fsrCorrected))',
        'extraInfo(looseMCMotherPDG)',
        'extraInfo(looseMCMotherIndex)',
        'extraInfo(looseMCWrongDaughterN)',
        'extraInfo(looseMCWrongDaughterPDG)',
        'extraInfo(looseMCWrongDaughterBiB)',
        ]

variablesToNTuple('e+:uncorrected', var0, filename='e_uncorrected.root')
variablesToNTuple('e+:corrected', var0, filename='e_corrected.root')
variablesToNTuple('J/psi:uncorrected', var1, filename='Jpsi_uncorrected.root')
variablesToNTuple('J/psi:corrected', var1, filename='Jpsi_corrected.root')
variablesToNTuple('J/psi:MC', var1, filename='Jpsi_MC.root')

# process the events
process(analysis_main)

# print out the summary
print(statistics)
