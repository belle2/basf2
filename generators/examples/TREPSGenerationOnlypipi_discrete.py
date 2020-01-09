#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run TREPS to generate e+e- -> e+e-pi+pi- events
#
# wListTable file is used and UseDiscreteAndSortedW is True.
# W distribution of generated events will be discrete and sorted.
# This is useful to hadronic two-photon analysis.
#
# Example steering file
########################################################

import basf2
from basf2 import set_log_level, LogLevel, process, statistics
from ROOT import Belle2
from beamparameters import add_beamparameters
from generators import add_treps_generator

set_log_level(LogLevel.DEBUG)

# main path
main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=99966)
main.add_module("Progress")

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")

# add TREPS generator
add_treps_generator(main, "e+e-pi+pi-", useDiscreteAndSortedW=True)

main.add_module("RootOutput", outputFileName="utrepsbpipi_100k_dicrete.root")
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.INFO, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
