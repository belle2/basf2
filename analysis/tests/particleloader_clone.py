#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A clone of the test of the ParticleLoader using a larger test file (mdst12.root)
this is not present on the bamboo server so this test only runs on buildbot or
wherever the validation-data are visible  (it's also a bit slower).
"""

import sys
import basf2
import b2test_utils

try:
    inputFile = basf2.find_file('mdst12.root', 'validation')
except FileNotFoundError as fnf:
    b2test_utils.skip_test("Cannot find: %s" % fnf.filename)

basf2.set_random_seed("1337")
fsps = ['e+', 'pi+', 'K+', 'p+', 'mu+', 'K_S0 -> pi+ pi-', 'Lambda0 -> p+ pi-', 'K_L0', 'gamma']

###############################################################################
# a new ParticleLoader for each fsp
testpath = basf2.create_path()
testpath.add_module('RootInput', inputFileName=inputFile, logLevel=basf2.LogLevel.ERROR)
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

testpath.add_module('ParticleStats', particleLists=fsps)
testpath.add_module('ParticleStats', particleLists=mcps)
basf2.process(testpath)

# process the first event (again) with the verbose ParticlePrinter
for fsp in fsps:
    testpath.add_module('ParticlePrinter', listName=fsp, fullPrint=True)
basf2.process(testpath, 1)
