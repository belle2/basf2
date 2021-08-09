#!/usr/bin/env python3

############################################################################
# basf2 (Belle II Analysis Software Framework)                             #
# Author: The Belle II Collaboration                                       #
#                                                                          #
# See git log for contributors and copyright holders.                      #
# This file is licensed under LGPL-3.0, see LICENSE.md.                    #
############################################################################

############################################################################
#                                                                          #
# Stuck? Ask for help at questions.belle2.org                              #
#                                                                          #
# This tutorial demonstrates how to estimate tracking systematic           #
# uncertainties for the following decay:                                   #
#                                                                          #
#   J/psi -> e+  e-                                                        #
#                                                                          #
# The analysers has to specify systematic "test" from the list of tests.   #
# The output file contains reconstructed J/psi inv. mass and momentum      #
# with corresponding modification.                                         #
#                                                                          #
# The latest recommendations for tracking systematic uncertainties can     #
# be found here:                                                           #
# https://confluence.desy.de/display/BI/Tracking+and+Vertexing+Performance #
#                                                                          #
############################################################################

import basf2 as b2
import modularAnalysis as ma

# define systematic tests
sys_tests = ['def', 'scale', 'scaleUp', 'scaleDown', 'Efficiency']

# Select the systematic variation from the list
test = 'scaleDown'

if test not in sys_tests:
    print(f"Unknown systematic test {test}")
    exit(1)

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('JPsi2ee_e2egamma.root', 'examples', False),
             path=my_path)


# fill particleLists
ma.fillParticleList(decayString='e+:sel',
                    cut='electronID > 0.2 and dr < 2 and abs(dz) < 4',
                    path=my_path)

if test == 'def':
    pass
elif test == 'scale':
    ma.scaleTrackMomenta(inputListNames=['e+:sel'], scale=1.00056, path=my_path)
elif test == 'scaleUp':
    ma.scaleTrackMomenta(inputListNames=['e+:sel'], scale=1.001, path=my_path)
elif test == 'scaleDown':
    ma.scaleTrackMomenta(inputListNames=['e+:sel'], scale=0.999, path=my_path)
elif test == 'Efficiency':
    ma.removeTracksForTrackingEfficiencyCalculation(inputListNames=['e+:sel'], fraction=0.01, path=my_path)

# J/psi
ma.reconstructDecay(decayString='J/psi:out -> e+:sel e-:sel',
                    cut='',
                    path=my_path)

var1 = ['M',
        'p',
        'E',
        ]

ma.variablesToNtuple(decayString='J/psi:out',
                     variables=var1,
                     filename=f'Jpsi_out_{test}.root',
                     path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
