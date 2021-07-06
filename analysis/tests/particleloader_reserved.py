#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Forbid users from filling an 'all' list with cuts. This has dangerous
potential to conflict with the standard lists and any standard tools.
'''

import b2test_utils
from basf2 import create_path, set_random_seed

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed('1337')
testinput = [b2test_utils.require_file('analysis/tests/mdst.root')]

# the name 'all' is reserved for the ParticleLoader
goodpath = create_path()
goodpath.add_module('RootInput', inputFileNames=testinput)
goodpath.add_module('ParticleLoader', decayStrings=['e+:all'])  # legal
b2test_utils.safe_process(goodpath, 1)

# it should throw b2fatal if there are cuts
badpath = create_path()
badpath.add_module('RootInput', inputFileNames=testinput)
badpath.add_module('ParticleLoader', decayStrings=['e+'])
badpath.add_module('ParticleListManipulator', outputListName='e+:my_electrons', inputListNames=['e+:all'])  # legal
badpath.add_module('ParticleListManipulator', outputListName='e+:all', inputListNames=['e+:my_electrons'])  # illlegal

b2test_utils.safe_process(badpath, 1)
