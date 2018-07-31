#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
a test of the ParticleLoader using the small test file for running quickly
there is a clone of this file which requires the presence of mdst7.root (a
larger file) that is present on the buildbot server
"""

from ROOT import Belle2
from basf2 import set_random_seed, create_path, process, LogLevel

set_random_seed("1337")
testinput = [Belle2.FileSystem.findFile('analysis/tests/mdst.root')]
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
