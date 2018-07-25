#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A clone of the test of the ParticleLoader using a larger test file (mdst7.root)
this is not present on the bamboo server so this test only runs on buildbot
(it's also a bit slower).
"""

import sys
from ROOT import Belle2
from basf2 import set_random_seed, create_path, process, LogLevel

set_random_seed("1337")
testinput = [Belle2.FileSystem.findFile('analysis/tests/mdst7.root')]
if len(testinput[0]) == 0:
    sys.stderr.write(
        "TEST SKIPPED: input file analysis/tests/mdst7.root"
        " not found. You can retrieve it via 'wget https://www.desy.de/~scunliff/mdst7.root'\n")
    sys.exit(-1)

fsps = ['e+', 'pi+', 'K+', 'p+', 'mu+', 'K_S0', 'Lambda0', 'K_L0', 'gamma']

###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput, logLevel=LogLevel.ERROR)
for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStringsWithCuts=[(fsp, '')])
testpath.add_module('ParticleStats', particleLists=fsps)
process(testpath)

# process the first event (again) with the verbose ParticlePrinter
for fsp in fsps:
    testpath.add_module('ParticlePrinter', listName=fsp, fullPrint=True)
process(testpath, 1)
