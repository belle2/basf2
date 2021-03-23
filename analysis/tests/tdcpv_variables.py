#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
a test of the TDCPV variables using the small test file for running quickly
"""

import b2test_utils
from basf2 import set_random_seed, create_path, process

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed("1337")
testinput = [b2test_utils.require_file('analysis/tests/mdst.root')]

fsps = ['mu-', 'pi-', 'gamma']
###############################################################################
# a new ParticleLoader for each fsp
testpath = create_path()
testpath.add_module('RootInput', inputFileNames=testinput)
for fsp in fsps:
    testpath.add_module('ParticleLoader', decayStrings=[fsp])

###############################################################################
signal_list = 'J/psi'
# Find J/psi
testpath.add_module('ParticleCombiner', decayString=f'{signal_list} -> mu+:all mu-:all',
                    cut='daughter(0, isSignal) > 0 and daughter(1, isSignal) > 0 '
                    ' and daughter(0, genMotherID) == daughter(1, genMotherID)')
# Signal vertex fit using kFit
testpath.add_module('ParticleVertexFitter', listName=signal_list,
                    confidenceLevel=0., vertexFitter='KFit')

testpath.add_module('ParticlePrinter', listName=signal_list, fullPrint=False)
# Build ROE
testpath.add_module('RestOfEventBuilder', particleList=signal_list,
                    particleListsInput=['pi+:all', 'gamma:all'])
# Tag vertex fit
testpath.add_module('TagVertex', listName=signal_list)
# Define ROE variables for testing
tdcpv_vars = ['DeltaT', 'DeltaTErr', 'DeltaTBelle', 'TagVz', 'TagVzErr']
# Print the variables to log
testpath.add_module('ParticlePrinter', listName=signal_list, fullPrint=False,
                    variables=tdcpv_vars)
process(testpath)
