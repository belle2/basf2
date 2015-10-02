#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run KKMC to generate uu events at KEKB-Belle energy
#
# Example steering file
########################################################

from basf2 import *
from ROOT import Belle2
from beamparameters import add_beamparameters

# suppress messages and warnings during processing:
set_log_level(LogLevel.WARNING)

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# beam parameters for KEKB-Belle
beamparameters = add_beamparameters(main, "KEKB-Belle")
# beamparameters.param("energyHER", 7.5) #KEKB-Belle at some arbitray value
# beamparameters.param("energyHER", 3.1) #KEKB-Belle at some arbitray value
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
print_params(beamparameters)

# use KKMC generated uubar pair as input
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/uubar_nohadronization.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', 'kkmc_uu.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput)
main.add_module("RootOutput", outputFileName="kkmc_uu.root")
main.add_module("PrintMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
