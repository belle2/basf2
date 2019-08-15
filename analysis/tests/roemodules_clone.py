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
fsps = ['K-', 'pi-', 'gamma', 'K_L0']

###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=inputFile)
for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStringsWithCuts=[(fsp, '')])
testpath.add_module('ParticleStats', particleLists=[fsps[0]])

testpath.add_module('RestOfEventBuilder', particleList=fsps[0],
                    particleListsInput=['pi+', 'gamma', 'K_L0'])
mask = ('cleanMask', 'E > 0.05', 'E > 0.05', [0, 0, 0, 0, 0, 0])
testpath.add_module('RestOfEventInterpreter', particleList=fsps[0],
                    ROEMasksWithFractions=mask)

###############################################################################
roe_path = create_path()
v0list = 'K_S0 -> pi+ pi-'
roe_path.add_module('ParticleLoader', decayStringsWithCuts=[(v0list, '')])

roe_path.add_module('RestOfEventUpdater',
                    particleList=v0list.split(' ->', 1)[0],
                    updateMasks=[mask[0]])
roe_path.add_module('RestOfEventPrinter',
                    maskNames=[mask[0]],
                    fullPrint=False)
testpath.for_each('RestOfEvent', 'RestOfEvents', path=roe_path)
###############################################################################

process(testpath, 1)
