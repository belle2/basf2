#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
testpath.add_module('ParticleLoader', decayStrings=fsps,
                    addDaughters=True, skipNonPrimaryDaughters=True, useMCParticles=True)
for fsp in fsps:
    testpath.add_module('ParticleListManipulator', outputListName=fsp,
                        inputListNames=[fsp + ':MC'], cut='mcPrimary > 0 and nDaughters == 0')

# Variables filled by event kinematics module
event_kinematics = [
    'genMissingMass2OfEvent',
    'genMissingEnergyOfEventCMS',
    'genMissingMomentumOfEventCMS',
    'genTotalPhotonsEnergyOfEvent',  # It is 0, because there is no primary photons in the event
    'genVisibleEnergyOfEventCMS',
]

testpath.add_module('EventKinematics', particleLists=fsps, usingMC=True)
# Print the variables to log
testpath.add_module('ParticlePrinter', listName='', fullPrint=False,
                    variables=event_kinematics)
process(testpath, 1)
