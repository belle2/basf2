#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run KKMC to generate tautau events
#
# Example steering file
########################################################

from basf2 import *
from ROOT import Belle2
from beamparameters import add_beamparameters

set_random_seed(12345)
set_log_level(LogLevel.INFO)

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=1000000)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)

# to run the framework the used modules need to be registered
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/tauola_orig.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
# kkgeninput.param('taudecaytableFile', Belle2.FileSystem.findFile('data/generators/kkmc/tau_decaytable.dat'))
# above line makes decay table to be read by Pythia; uncomment next line to make tau decay table to be read by Tauola
kkgeninput.param('taudecaytableFile', ' ')
kkgeninput.param('kkmcoutputfilename', 'kkmc_tautau.txt')

# run
main.add_module("Progress")
main.add_module(kkgeninput)
# main.add_module("RootOutput", outputFileName="kkmc_tautau_orig.root")
main.add_module("HepMCOutput", OutputFilename='kkmc_tautau_orig.hepmc', StoreVirtualParticles=True)
# main.add_module("PrintTauTauMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)
# main.add_module("PrintMCParticles", logLevel=LogLevel.INFO, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
