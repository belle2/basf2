#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
A test of the ParticleLoader using a relatively large test file
(mdst14.root in the validation data)
"""

import basf2
import b2test_utils

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
basf2.set_random_seed("1337")

fsps = ['e+', 'pi+', 'K+', 'p+', 'mu+', 'K_S0 -> pi+ pi-', 'Lambda0 -> p+ pi-', 'K_L0', 'gamma', 'n0']

###############################################################################
# a new ParticleLoader for each fsp
testpath = basf2.create_path()
inputFile = b2test_utils.require_file('mdst14.root', 'validation')
testpath.add_module('RootInput', inputFileName=inputFile)
for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStrings=[fsp])
testpath.add_module('ParticleListManipulator', outputListName='gamma',
                    inputListNames=['gamma:all'], cut='isFromECL')

# manipulate the string to remove the daughters in case of v0
for i in range(len(fsps)):
    if " -> " in fsps[i]:
        fsps[i] = fsps[i].split(' ->', 1)[0]

# also load MC particles
mcps = [particle + ':MC' for particle in fsps + ['B0', 'D0']]
testpath.add_module('ParticleLoader', decayStrings=mcps, useMCParticles=True)

# load photons from KLMCluster
testpath.add_module('ParticleListManipulator', outputListName='gamma:fromKLM',
                    inputListNames=['gamma:all'], cut='isFromKLM')

# add RestOfEvents
signal_side = 'K_S0:V0'
roe_side = 'Upsilon(4S):ROE'
testpath.add_module('RestOfEventBuilder', particleList=signal_side,
                    particleListsInput=['pi+:all', 'gamma', 'K_L0:all'])
# Load RestOfEvents
testpath.add_module('ParticleLoader', decayStrings=[roe_side],
                    sourceParticleListName=signal_side, useROEs=True)

for i in range(len(fsps)):
    if 'K_S0' in fsps[i] or 'Lambda0' in fsps[i]:
        fsps[i] = fsps[i] + ':V0'
    elif "gamma" not in fsps[i]:
        fsps[i] = fsps[i] + ':all'

testpath.add_module('ParticleStats', particleLists=fsps)
testpath.add_module('ParticleStats', particleLists=mcps)
testpath.add_module('ParticleStats', particleLists=['gamma:fromKLM'])
testpath.add_module('ParticleStats', particleLists=[roe_side])
basf2.process(testpath)

# process the first event (again) with the verbose ParticlePrinter
for fsp in fsps:
    testpath.add_module('ParticlePrinter', listName=fsp, fullPrint=True)
testpath.add_module('ParticlePrinter', listName='gamma:fromKLM', fullPrint=True)
basf2.process(testpath, 1)
