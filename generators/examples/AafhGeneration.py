#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *
import os
import sys

# suppress messages and during processing:
set_log_level(LogLevel.WARNING)

# set event info for generated events
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])  # we want to process 100 events

# load parameters (i.e. beam energies)
gearbox = register_module('Gearbox')

aafh = register_module('AafhInput')
aafh.param({
    # decay mode to generate.
    # 1: e+e- -> mu+mu-L+L- where L is a user defined particle (default: tau)
    # 2: e+e- -> mu+mu-mu+mu-
    # 3: e+e- -> e+e-mu+mu-
    # 4: e+e- -> e+e-L+L- where L is a user defined particle (default: tau)
    # 5: e+e- -> e+e-e+e-
    # to set the particle for modes 1 and 4 use set parameter "particle"
    "mode": 5,
    # rejection scheme to generate unweighted events
    # 1: use rejection once for the final event weight
    # 2: use rejection per sub generator and then for the final event
    "rejection": 2,
    # max subgenerator event weight, only used if rejection is set to 2
    # (default). If this value is to low the generation will produce errors. If
    # it is to high generation runs slower.
    "maxSubgeneratorWeight": 1.,
    # max final event weight which is always used. If this value is to low the
    # generation will produce errors. If it is to high generation runs slower.
    "maxFinalWeight": 2.,
    # adjust subgenerator weights so that each sub generator has same
    # probability to be called and the maximum weight is equal as well. These
    # values are printed at the end of generation when output level is set to
    # INFO. These weights strongly depend on the mode
    "subgeneratorWeights": [1.000e+00, 3.173e+02, 1.602e+10, 3.230e+10,
                            1.000e+00, 9.598e-01, 2.564e+00, 5.440e+00],
    # set to awfully precise
    "suppressionLimits": [1e100] * 4,
})
aafh.logging.log_level = LogLevel.INFO

# print generated particles
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.INFO

# creating the path for the processing
main = create_path()
# and all modules we created
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(aafh)
main.add_module(mcparticleprinter)

# process the events
process(main)
