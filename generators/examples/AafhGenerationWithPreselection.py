#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import os
import sys

# suppress messages and during processing:
set_log_level(LogLevel.INFO)

# set event info for generated events
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])  # we want to process 10000 events

# load parameters (i.e. beam energies)
gearbox = register_module('Gearbox')

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
    'mode': 5,
    'minMass': 0.50,
    'rejection': 2,
    'maxSubgeneratorWeight': 1.0,
    'maxFinalWeight': 3.0,
    'subgeneratorWeights': [
        1.0,
        7.986e+01,
        5.798e+04,
        3.898e+05,
        1.0,
        1.664e+00,
        2.812e+00,
        7.321e-01,
        ],
    'suppressionLimits': [1e100] * 4,
    })
aafh.logging.log_level = LogLevel.INFO

# preselection to reject no tag events
generatorpreselection = register_module('GeneratorPreselection')
generatorpreselection.param('nChargedMin', 1)
# generatorpreselection.param('nChargedMax', 999)
generatorpreselection.param('MinChargedP', 0.25)
generatorpreselection.param('MinChargedPt', 0.1)
generatorpreselection.param('MinChargedTheta', 17.)
generatorpreselection.param('MaxChargedTheta', 150.)
generatorpreselection.param('nPhotonMin', 1)
# generatorpreselection.param('nPhotonMax', 999)
generatorpreselection.param('MinPhotonEnergy', 0.50)
generatorpreselection.param('MinPhotonTheta', 15.)
generatorpreselection.param('MaxPhotonTheta', 170.)

# print generated particles
mcparticleprinter = register_module('PrintMCParticles')
mcparticleprinter.logging.log_level = LogLevel.INFO

output = register_module('RootOutput')
output.param('outputFileName', './aafh_out.root')

# creating the path for the processing
main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(aafh)
main.add_module(generatorpreselection)
# create an empty path and check the return value of the the preselection
emptypath = create_path()
generatorpreselection.if_value('<1', emptypath)
# continue the main path otherwise (e.g. call detector simulation)
main.add_module(mcparticleprinter)
main.add_module(output)

# process the events
process(main)

# show call statistics
print statistics

