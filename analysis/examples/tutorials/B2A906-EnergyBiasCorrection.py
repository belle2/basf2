#!/usr/bin/env python3

import basf2 as b2
import modularAnalysis as ma
import variables as va
import os

# defile scale tests
sys_tests = ['def', 'scale']

# Select the variation from the list
test = 'def'

if test not in sys_tests:
    print("Unknown systematic test {}".format(test))
    exit(1)

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)

# ID of weight table is taken from B2A907
weight_table_id = "PhotonEnergyBiasCorrection_Feb2021:TestEnergy"

if not os.getenv('BELLE2_EXAMPLES_DATA_DIR'):
    b2.B2FATAL("You need the example data installed. Run `b2install-data example` in terminal for it.")

db_location = os.getenv('BELLE2_EXAMPLES_DATA_DIR') + '/database/'
b2.conditions.prepend_testing_payloads('localdb/database.txt')

# We know what weight info will be added (see B2A907),
# so we add aliases and add it to the tools
va.variables.addAlias('Weight', 'extraInfo(' + weight_table_id + '_Weight)')
va.variables.addAlias('StatErr', 'extraInfo(' + weight_table_id + '_StatErr)')
va.variables.addAlias('binID', 'extraInfo(' + weight_table_id + '_binID)')

# fill particleLists
GammaSelection = 'theta > 0.296706 and theta < 2.6179'
ma.fillParticleList(decayString='gamma:sel',
                    cut=GammaSelection,
                    path=my_path)

if test == 'def':
    pass
elif test == 'scale':  # weight_table_id+"_Weight" is used to scale photon energy
    ma.energyBiasCorrection(inputListNames=['gamma:sel'], tableName=weight_table_id, path=my_path)

var1 = ['M',
        'p',
        'E', 'Weight', 'StatErr', 'binID'
        ]


ma.variablesToNtuple(decayString='gamma:sel',
                     variables=var1,
                     filename='gamma_{}.root'.format(test),
                     path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
