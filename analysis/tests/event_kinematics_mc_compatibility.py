#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
a test of the EventKinematics variables using the small test file for running quickly
"""

import b2test_utils
from basf2 import set_random_seed, create_path, process

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed("1337")
testinput = [b2test_utils.require_file('analysis/tests/mdst.root')]

fsps = ['gamma', 'e+', 'mu+', 'pi+', 'K+', 'p+',
                 'K_S0', 'Lambda0']
###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput)
for fsp in fsps:
    # There is an annoying warning in ParticleLoader about zero charged tracks
    # If it will be removed in the future releases, the test log should be
    # updated respectively.
    testpath.add_module('ParticleLoader', decayStringsWithCuts=[(fsp, 'mcPrimary > 0 and nDaughters == 0')],
                        addDaughters=True, skipNonPrimaryDaughters=True, useMCParticles=True)

# Variables created by event kinematics module
event_kinematics = [
    'genMissingMass2OfEvent',
    'genMissingEnergyOfEventCMS',
    'genMissingMomentumOfEventCMS'
]

testpath.add_module('EventKinematics', particleLists=fsps, fromMC=True)
# Print the variables to log
testpath.add_module('ParticlePrinter', listName='', fullPrint=False,
                    variables=event_kinematics)
process(testpath, 1)
