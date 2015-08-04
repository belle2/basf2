#!/usr/bin/env python
# -*- coding: utf-8 -*-
########################################################
# 100 eemm events are generated using the AAFH
# generator
#
# Example steering file
########################################################

from basf2 import *
from beamparameters import add_beamparameters
import os
import sys

# suppress messages and during processing:
set_log_level(LogLevel.WARNING)

main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
beamparameters.param("smearVertex", True)
beamparameters.param("generateCMS", False)
# print_params(beamparameters)

# generator
aafh = register_module('AafhInput')
aafh.param({  # decay mode to generate.
              # 1: e+e- -> mu+mu-L+L- where L is a user defined particle (default: tau)
              # 2: e+e- -> mu+mu-mu+mu-
              # 3: e+e- -> e+e-mu+mu-
              # 4: e+e- -> e+e-L+L- where L is a user defined particle (default: tau)
              # 5: e+e- -> e+e-e+e-
              # to set the particle for modes 1 and 4 use set parameter "particle"
              # rejection scheme to generate unweighted events
              # 1: use rejection once for the final event weight
              # 2: use rejection per sub generator and then for the final event
              # max subgenerator event weight, only used if rejection is set to 2
              # (default). If this value is to low the generation will produce errors. If
              # it is to high generation runs slower.
              # max final event weight which is always used. If this value is to low the
              # generation will produce errors. If it is to high generation runs slower.
              # ==> should be around 2-4
              # adjust subgenerator weights so that each sub generator has same
              # probability to be called and the maximum weight is equal as well. These
              # values are printed at the end of generation when output level is set to
              # INFO. These weights strongly depend on the mode
              # set to awfully precise
    'mode': 3,
    'minMass': 0.50,
    'rejection': 2,
    'maxSubgeneratorWeight': 1.0,
    'maxFinalWeight': 1.5,
    'subgeneratorWeights': [1.000e+00, 2.216e+01, 3.301e+03, 6.606e+03, 1.000e+00, 1.675e+00, 5.948e+00, 6.513e+00],
    'suppressionLimits': [1e100] * 4,
})
aafh.logging.log_level = LogLevel.INFO

# print generated particles
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.INFO

# creating the path for the processing
main.add_module(aafh)
main.add_module(mcparticleprinter)

# process the events
process(main)
