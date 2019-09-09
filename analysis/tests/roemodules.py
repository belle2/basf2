#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
a test of the ROE-related modules using the small test file for running quickly
there is a clone of this file which requires the presence of mdst12.root (a
larger file) that is present on the buildbot server but not bamboo
"""

import b2test_utils
from basf2 import set_random_seed, create_path, process, conditions

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
conditions.disable_globaltag_replay()
set_random_seed("1337")
testinput = [b2test_utils.require_file('analysis/tests/mdst.root')]
fsps = ['K-', 'pi-', 'gamma', 'K_L0']

###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput)
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
