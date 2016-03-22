#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 10 generic BBbar events using EvtGen incl. FullSim
#
# Example steering file
########################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# to run the framework the used modules need to be registered
evtgen = register_module('EvtGenInput')
evtgen.set_log_level(LogLevel.INFO)

# run
main.add_module("Progress")
main.add_module("Gearbox")
main.add_module("Geometry")
main.add_module(evtgen)
main.add_module("FullSim")
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)
main.add_module("RootOutput", outputFileName="evtgen_upsilon4s_fullsim.root")

# Process the events
process(main)

# show call statistics
print(statistics)
