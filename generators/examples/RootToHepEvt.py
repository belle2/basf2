#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Convert basf2 Root output to HepEvt
#
# Example steering file
########################################################

import basf2

# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

# Register the input module
myinput = basf2.register_module('RootInput')
myinput.param('inputFileName', './input.root')

# Register the output module
myoutput = basf2.register_module('HepevtOutput')
myoutput.param('OutputFilename', './output.hepevt')

# Register the Progress module and the Python histogram module
myprogress = basf2.register_module('Progress')

main.add_module(myprogress)
main.add_module(myinput)
main.add_module(myoutput)
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
