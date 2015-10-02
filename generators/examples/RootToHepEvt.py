#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Convert basf2 Root output to HepEvt
#
# Example steering file
########################################################

from basf2 import *

# Set the global log level
set_log_level(LogLevel.INFO)

main = create_path()

# Register the input module
myinput = register_module('RootInput')
myinput.param('inputFileName', './input.root')

# Register the output module
myoutput = register_module('HepevtOutput')
myoutput.param('OutputFilename', './output.hepevt')

# Register the Progress module and the Python histogram module
myprogress = register_module('Progress')

main = create_path()
main.add_module(myprogress)
main.add_module(myinput)
main.add_module(myoutput)
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
