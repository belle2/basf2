#!/usr/bin/env python3

import basf2 as b2
import modularAnalysis as ma

# defile systematic tests
sys_tests = ['def', 'scale', 'scaleUp', 'scaleDown']

# Select the systematic variation from the list
test = 'scaleDown'

if test not in sys_tests:
    print("Unknown systematic test {}".format(test))
    exit(1)

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2rhogamma_rho2pipi.root', 'examples', False),
             path=my_path)


# fill particleLists
GammaSelection = 'theta > 0.296706 and theta < 2.6179'
ma.fillParticleList(decayString='gamma:sel',
                    cut=GammaSelection,
                    path=my_path)

if test == 'def':
    pass
elif test == 'scale':
    ma.energyBiasCorrection(inputListNames=['gamma:sel'], scale=1.007, path=my_path)
elif test == 'scaleUp':
    ma.energyBiasCorrection(inputListNames=['gamma:sel'], scale=1.008, path=my_path)
elif test == 'scaleDown':
    ma.energyBiasCorrection(inputListNames=['gamma:sel'], scale=1.006, path=my_path)


var1 = ['M',
        'p',
        'E',
        ]

ma.variablesToNtuple(decayString='gamma:sel',
                     variables=var1,
                     filename='gamma_{}.root'.format(test),
                     path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
