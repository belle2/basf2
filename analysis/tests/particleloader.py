#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
a test of the ParticleLoader using the small test file for running quickly
there is a clone of this file which requires the presence of mdst12.root (a
larger file) that is present on the buildbot server but not bamboo
"""

from ROOT import Belle2
from basf2 import set_random_seed, create_path, process, LogLevel

set_random_seed("1337")
testinput = [Belle2.FileSystem.findFile('analysis/tests/mdst.root')]
fsps = ['e+', 'pi+', 'K+', 'p+', 'mu+', 'K_S0 -> pi+ pi-', 'Lambda0 -> p+ pi-', 'K_L0', 'gamma']

###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput, logLevel=LogLevel.ERROR)
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
process(testpath)

# process the first event (again) with the verbose ParticlePrinter
for fsp in fsps:
    testpath.add_module('ParticlePrinter', listName=fsp, fullPrint=True)
process(testpath, 1)
