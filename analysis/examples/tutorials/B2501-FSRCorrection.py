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
#               I. Komarov (Demeber 2017)
#
######################################################

from basf2 import *
from modularAnalysis import *

# set the log level
set_log_level(LogLevel.WARNING)

# Bd_JpsiKL_ee Signal MC file
inputFile = "/hsm/belle/bdata2/users/ikomarov/tutorial_samples/1111540100.dst.root"

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

# Please note, a new lepton is generated, with the old electron and -if found- a gamma as daughters.
# Information attached to the track is only available for the old lepton, accessable via the daughter
# metavariable, e.g. <daughter(0, eid)>.

var0 = ['p',
        'px',
        'py',
        'pz',
        'x',
        'y',
        'z',
        'daughter(0, electronID)',
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
