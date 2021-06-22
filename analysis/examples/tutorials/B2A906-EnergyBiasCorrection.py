#!/usr/bin/env python3

import basf2 as b2
import modularAnalysis as ma
import variables as va
import os

# define scale tests
sys_tests = ['def', 'scale']

# Select the variation from the list
test = 'scale'

if test not in sys_tests:
    print(f"Unknown systematic test {test}")
    exit(1)

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)

# ID of weight table
weight_table_id = "PhotonEnergyBiasCorrection_Feb2021"

b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# We know what weight info will be added,
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
elif test == 'scale':
    # The scaling must only be applied to collision data, but not to MC.
    # In principle, it can be run for MC as well because internally MC events are skipped anyway.
    # However, it's better (faster, no error message) to skip the energy scaling in the steering file when running over MC.
    # weight_table_id+"_Weight" is used to scale photon energy
    ma.correctEnergyBias(inputListNames=['gamma:sel'], tableName=weight_table_id, path=my_path)

var1 = ['M',
        'p',
        'E', 'Weight', 'StatErr', 'binID'
        ]


ma.variablesToNtuple(decayString='gamma:sel',
                     variables=var1,
                     filename=f'gamma_{test}.root',
                     path=my_path)

# process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
