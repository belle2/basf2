#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 100 Upsilon(1) events using EvtGen
#
# Example steering file
########################################################

from basf2 import *
from beamparameters import add_beamparameters

# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# beam parameters
beamparameters = add_beamparameters(main, "Y1S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
# beamparameters.param("smearEnergy", False)
# print_params(beamparameters)

# EvtGen
evtgen = register_module('EvtGenInput')
# parent particle name from evt.pdl: Upsilon = Y(1S)
evtgen.param('ParentParticle', "Upsilon")
evtgen.set_log_level(LogLevel.INFO)

# run
main.add_module("Progress")
main.add_module("Gearbox")
main.add_module(evtgen)
main.add_module("RootOutput", outputFileName="evtgen_upsilon1s.root")
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
