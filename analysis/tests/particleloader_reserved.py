#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
Forbid users from filling an 'all' list with cuts. This has dangerous
potential to conflict with the standard lists and any standard tools.
'''

import b2test_utils
from basf2 import set_random_seed, create_path, process

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed('1337')
testinput = [b2test_utils.require_file('analysis/tests/mdst.root')]

# the name 'all' is reserved, but it should still work as long as it really is all
goodpath = create_path()
goodpath.add_module('RootInput', inputFileNames=testinput)
goodpath.add_module('ParticleLoader', decayStringsWithCuts=[('e+:all', '')])  # legal
b2test_utils.safe_process(goodpath, 1)

# it should throw b2fatal if there are cuts
badpath = create_path()
badpath.add_module('RootInput', inputFileNames=testinput)
badpath.add_module('ParticleLoader', decayStringsWithCuts=[('e+:all', 'p > 3')])  # illegal
b2test_utils.safe_process(badpath, 1)
