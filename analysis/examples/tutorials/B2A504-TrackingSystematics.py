#!/usr/bin/env python3

###############################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to estimate tracking systematic uncertainties for
# the following decay:
#
#   J/psi -> e+  e-
#
# The analysers has to specify systematic "test" from the list of tests. The output
# file contains reconstructed J/psi inv. mass and momentum with corresponding modification.
#
# The latest recommendations for tracking systematic uncertainties can be found here:
# https://confluence.desy.de/display/BI/Tracking+and+Vertexing+Performance
#
###############################################################################

import basf2 as b2
import modularAnalysis as ma

# defile systematic tests
sys_tests = ['def', 'scale', 'scaleUp', 'scaleDown', 'Efficiency']

# Select the systematic variation from the list
test = 'scaleDown'

if test not in sys_tests:
    print("Unknown systematic test {}".format(test))
    exit(1)

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('JPsi2ee_e2egamma.root', 'examples', False),
             path=my_path)


# fill particleLists
ma.fillParticleList(decayString='e+:sel',
                    cut='electronID > 0.2 and dr < 2 and abs(dz) < 4',
                    path=my_path)

if test == 'def':
    pass
elif test == 'scale':
    ma.trackingMomentum(inputListNames=['e+:sel'], scale=1.00056, path=my_path)
elif test == 'scaleUp':
    ma.trackingMomentum(inputListNames=['e+:sel'], scale=1.001, path=my_path)
elif test == 'scaleDown':
    ma.trackingMomentum(inputListNames=['e+:sel'], scale=0.999, path=my_path)
elif test == 'Efficiency':
    ma.trackingEfficiency(inputListNames=['e+:sel'], fraction=0.01, path=my_path)

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
                     filename='Jpsi_out_{}.root'.format(test),
                     path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
