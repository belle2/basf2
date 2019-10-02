#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A clone of the test of the ParticleLoader using a larger test file (mdst12.root)
this is not present on the bamboo server so this test only runs on buildbot or
wherever the validation-data are visible  (it's also a bit slower).
"""

import basf2
import b2test_utils

inputFile = b2test_utils.require_file('mdst12.root', 'validation')
# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
basf2.conditions.disable_globaltag_replay()
basf2.set_random_seed("1337")
fsps = ['e+', 'pi+', 'K+', 'p+', 'mu+', 'K_S0 -> pi+ pi-', 'Lambda0 -> p+ pi-', 'K_L0', 'gamma', 'n0']

###############################################################################
# a new ParticleLoader for each fsp
testpath = basf2.create_path()
testpath.add_module('RootInput', inputFileName=inputFile)
for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStringsWithCuts=[(fsp, '')])

# manipulate the string to remove the daughters in case of v0
for i in range(len(fsps)):
    if " -> " in fsps[i]:
        fsps[i] = fsps[i].split(' ->', 1)[0]

# also load MC particles
mcps = [particle + ':frommc' for particle in fsps + ['B0', 'D0']]
for mcp in mcps:
    testpath.add_module('ParticleLoader', decayStringsWithCuts=[(mcp, '')],
                        useMCParticles=True)
# add RestOfEvents
signal_side = 'K_S0'
roe_side = 'Upsilon(4S)'
testpath.add_module('RestOfEventBuilder', particleList=signal_side,
                    particleListsInput=['pi+', 'gamma', 'K_L0'])
# Load RestOfEvents
testpath.add_module('ParticleLoader', decayStringsWithCuts=[(roe_side, '')],
                    sourceParticleListName=signal_side, useROEs=True)


testpath.add_module('ParticleStats', particleLists=fsps)
testpath.add_module('ParticleStats', particleLists=mcps)
testpath.add_module('ParticleStats', particleLists=[roe_side])
basf2.process(testpath)

# process the first event (again) with the verbose ParticlePrinter
for fsp in fsps:
    testpath.add_module('ParticlePrinter', listName=fsp, fullPrint=True)
basf2.process(testpath, 1)
