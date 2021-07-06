#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to correct for Bremsstrahlung radiation #
# for the following decay:                                               #
#                                                                        #
#   J/psi                                                                #
#    |                                                                   #
#    +-> e- e+                                                           #
#        |  |                                                            #
#        |  +-> e+ (gamma)                                               #
#        +-> e- (gamma)                                                  #
#                                                                        #
# Note:                                                                  #
#                                                                        #
# The electrons in the corrected list have an attached extraInfo         #
# 'bremsCorrected', indicating if the electron was corrected (1) or not  #
#  (0).                                                                  #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('JPsi2ee_e2egamma.root', 'examples', False),
             path=my_path)


# fill particleLists
ma.fillParticleList(decayString='e+:uncorrected',
                    cut='electronID > 0.2 and dr < 2 and abs(dz) < 4',
                    path=my_path)
ma.fillParticleList(decayString='gamma:bremsinput',
                    cut='E < 1.0',
                    path=my_path)

# MC matching
ma.matchMCTruth(list_name='e+:uncorrected', path=my_path)
ma.matchMCTruth(list_name='gamma:bremsinput', path=my_path)

# correction of Bremsstrahlung
# A new lepton is generated, with the old electron and, if found, a gamma as daughters.
ma.correctBrems(outputList='e+:corrected',
                inputList='e+:uncorrected',
                gammaList='gamma:bremsinput',
                path=my_path)
ma.matchMCTruth(list_name='e+:corrected',
                path=my_path)

# uncorrected
ma.reconstructDecay(decayString='J/psi:uncorrected -> e+:uncorrected e-:uncorrected',
                    cut='',
                    path=my_path)
ma.reconstructDecay(decayString='J/psi:corrected -> e+:corrected e-:corrected',
                    cut='',
                    path=my_path)

# MC matching
ma.matchMCTruth(list_name='J/psi:uncorrected', path=my_path)
ma.matchMCTruth(list_name='J/psi:corrected', path=my_path)

# get all MC particles
ma.fillParticleListFromMC(decayString='J/psi:MC', cut="", path=my_path)

# write out ntuples

var0 = ['p',
        'px',
        'py',
        'pz',
        'x',
        'y',
        'z',
        'electronID',
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
        'extraInfo(bremsCorrected)'
        ]
var1 = ['M',
        'p',
        'E',
        'x', 'y', 'z',
        'isSignal',
        'mcErrors',
        'daughter(0, p)',
        'daughter(1, p)',
        'daughter(0, extraInfo(bremsCorrected))',
        'daughter(1, extraInfo(bremsCorrected))'
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
