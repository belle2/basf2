#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run KKMC to generate tautau events
#
# Example steering file
########################################################

import basf2
from ROOT import Belle2
from beamparameters import add_beamparameters

basf2.set_log_level(basf2.LogLevel.INFO)

# main path
main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)

# to run the framework the used modules need to be registered
kkgeninput = basf2.register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/tau.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', Belle2.FileSystem.findFile('data/generators/kkmc/tau_decaytable.dat'))
kkgeninput.param('kkmcoutputfilename', 'kkmc_tautau.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput)
main.add_module("RootOutput", outputFileName="kkmc_tautau.root")
# main.add_module("PrintTauTauMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.INFO, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
