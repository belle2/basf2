#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A clone of the test of the ROE-related module tests using a larger test file (mdst12.root)
this is not present on the bamboo server so this test only runs on buildbot or
wherever the validation-data are visible  (it's also a bit slower).
"""

import b2test_utils
from basf2 import set_random_seed, create_path, process

inputFile = b2test_utils.require_file('mdst12.root', 'validation')
# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed("1337")
fsps = ['K-:all', 'pi-:all', 'gamma:all', 'K_L0:all']

###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=inputFile)
for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStrings=[fsp])
testpath.add_module('ParticleSelector', decayString='gamma:all', cut='isFromECL')
testpath.add_module('ParticleStats', particleLists=[fsps[0]])

testpath.add_module('RestOfEventBuilder', particleList=fsps[0],
                    particleListsInput=['pi+:all', 'gamma:all', 'K_L0:all'])
mask = ('cleanMask', 'E > 0.05', 'E > 0.05', '')
testpath.add_module('RestOfEventInterpreter', particleList=fsps[0],
                    ROEMasks=mask)

###############################################################################
roe_path = create_path()
v0list = 'K_S0:V0 -> pi+ pi-'
roe_path.add_module('ParticleLoader', decayStrings=[v0list])

roe_path.add_module('ParticleLoader', decayStrings=['mu+'])
roe_path.add_module('ParticleListManipulator', outputListName='mu+:roe',
                    inputListNames=['mu+:all'], cut='isInRestOfEvent == 1 and isSignal == 1')

roe_path.add_module('RestOfEventUpdater',
                    particleList=v0list.split(' ->', 1)[0],
                    updateMasks=[mask[0]])
roe_path.add_module('RestOfEventPrinter',
                    maskNames=[mask[0]],
                    fullPrint=False)

jpsi_roe_list = 'J/psi:roe'
roe_path.add_module('ParticleCombiner',
                    decayString=jpsi_roe_list + ' -> mu+:roe mu-:roe',
                    cut='')
roe_path.add_module('ParticlePrinter', listName=jpsi_roe_list, fullPrint=False)

roe_path.add_module('RestOfEventBuilder', particleList=jpsi_roe_list,
                    createNestedROE=True, nestedROEMask=mask[0])

# --------------------------------------------------------------------------- #
nested_roe_path = create_path()

nested_roe_path.add_module('RestOfEventPrinter',
                           fullPrint=False)

roe_path.for_each('RestOfEvent', 'NestedRestOfEvents', path=nested_roe_path)
# --------------------------------------------------------------------------- #

nested_list = 'B+:other'
roe_path.add_module('ParticleLoader',
                    decayStrings=[nested_list + ' -> ' + jpsi_roe_list],
                    useROEs=True)
roe_path.add_module('ParticleListManipulator',
                    outputListName=nested_list,
                    inputListNames=['B+:ROE'])

roe_path.add_module('ParticleStats', particleLists=[nested_list])
roe_path.add_module('ParticlePrinter', listName=nested_list, fullPrint=True)

testpath.for_each('RestOfEvent', 'RestOfEvents', path=roe_path)
###############################################################################

process(testpath, 5)
