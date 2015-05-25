#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
set_log_level(LogLevel.ERROR)

# to run the framework the used modules need to be registered
kkgeninput = register_module('KKGenInput')
kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
kkgeninput.param('taudecaytableFile', '')
kkgeninput.param('kkmcoutputfilename', './KKGenMuon_out.txt')

eventinfosetter = register_module('EventInfoSetter')
gearbox = register_module('Gearbox')

eventinfosetter.param('evtNumList', [10])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# output
output = register_module('RootOutput')
output.param('outputFileName', './KKGenMuon_out.root')

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(progress)
main.add_module(kkgeninput)
main.add_module(output)

# uncomment the following line if you want event by event info
# main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print statistics
