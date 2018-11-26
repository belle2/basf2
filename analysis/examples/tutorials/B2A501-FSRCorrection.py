#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
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
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('JPsi2ee_e2egamma.root', 'examples', False),
             path=my_path)


# fill particleLists
ma.fillParticleList(decayString='e+:uncorrected',
                    cut='electronID > 0.2 and d0 < 2 and abs(z0) < 4',
                    path=my_path)
ma.fillParticleList(decayString='gamma:all',
                    cut='E < 1.0',
                    writeOut=False,
                    path=my_path)

# loose mc matching (recommended)
ma.looseMCTruth(list_name='e+:uncorrected', path=my_path)
ma.looseMCTruth(list_name='gamma:all', path=my_path)

# fsr correction
ma.correctFSR(outputListName='e+:corrected',
              inputListName='e+:uncorrected',
              gammaListName='gamma:all',
              angleThreshold=5.0,
              energyThreshold=1.0,
              writeOut=False,
              path=my_path)
ma.looseMCTruth(list_name='e+:corrected',
                path=my_path)

# uncorrected
ma.reconstructDecay(decayString='J/psi:uncorrected -> e+:uncorrected e-:uncorrected',
                    cut='',
                    path=my_path)
ma.reconstructDecay(decayString='J/psi:corrected -> e+:corrected e-:corrected',
                    cut='',
                    path=my_path)

# loose MC matching
ma.looseMCTruth(list_name='J/psi:uncorrected', path=my_path)
ma.looseMCTruth(list_name='J/psi:corrected', path=my_path)

# get all MC particles
ma.fillParticleListFromMC(decayString='J/psi:MC', cut="", path=my_path)

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

ma.variablesToNtuple(decayString='e+:uncorrected',
                     variables=var0,
                     filename='e_uncorrected.root',
                     path=my_path)
ma.variablesToNtuple(decayString='e+:corrected',
                     variables=var0,
                     filename='e_corrected.root',
                     path=my_path)
ma.variablesToNtuple(decayString='J/psi:uncorrected',
                     variables=var1,
                     filename='Jpsi_uncorrected.root',
                     path=my_path)
ma.variablesToNtuple(decayString='J/psi:corrected',
                     variables=var1,
                     filename='Jpsi_corrected.root',
                     path=my_path)
ma.variablesToNtuple(decayString='J/psi:MC',
                     variables=var1,
                     filename='Jpsi_MC.root',
                     path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
