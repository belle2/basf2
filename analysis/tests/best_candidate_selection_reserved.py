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
Forbid users from using the Best Candidate Selection module to remove candidates from particle lists with protected identifiers.
'''

import b2test_utils
from basf2 import create_path, set_random_seed
import modularAnalysis as ma

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed('1337')
testinput = [b2test_utils.require_file('analysis/tests/mdst.root')]

# the particle list with identifier 'all' cannot be modified
badpath_all = create_path()
badpath_all.add_module('RootInput', inputFileNames=testinput)
badpath_all.add_module('ParticleLoader', decayStrings=['e+:all'])
ma.rankByLowest("e+:all", "px", numBest=0, path=badpath_all)  # legal
ma.rankByLowest("e+:all", "px", numBest=1, path=badpath_all)  # illegal
b2test_utils.safe_process(badpath_all, 1)

# same for identifier 'MC'
badpath_MC = create_path()
badpath_MC.add_module('RootInput', inputFileNames=testinput)
ma.fillParticleListFromMC("e-:MC", "", path=badpath_MC)
ma.rankByLowest("e+:MC", "px", numBest=0, path=badpath_MC)  # legal
ma.rankByLowest("e+:MC", "px", numBest=1, path=badpath_MC)  # illegal
b2test_utils.safe_process(badpath_MC, 1)

# and identifier V0
badpath_V0 = create_path()
badpath_V0.add_module('RootInput', inputFileNames=testinput)
ma.fillParticleList('K_S0:V0 -> pi+ pi-', '', True, path=badpath_V0)
ma.rankByLowest("K_S0:V0", "px", numBest=0, path=badpath_V0)  # legal
ma.rankByLowest("K_S0:V0", "px", numBest=1, path=badpath_V0)  # illegal
b2test_utils.safe_process(badpath_V0, 1)
