#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This steering file demonstrates the use of the EventRandomizer module.
# It reads in a beam background simulation file and saves back its events in
# random order.
#
##############################################################################

from basf2 import *

# show warnings during processing
set_log_level(LogLevel.ERROR)

inputDir = '~/work/belle2/BG/fall2012'
# A single background file !
inputName = '{d}/Touschek_LER_all.root'.format(d=inputDir)

outputDir = '.'
outputName = '{d}/Touschek_LER_rnd.root'.format(d=outputDir)
# ****************************************************************************

# Register modules

# ROOTInput module
rootinput = register_module('RootInput')
# CHANGE THIS TO POINT TO THE APPROPRIATE FILE(S) ON YOUR FILESYSTEM!!!
rootinput.param('inputFileName', inputName)

# rootinput.param('excludeBranchNames',['TOPTracks'])
# rootinput.set_log_level(LogLevel.INFO)
# EventRandomizer module
randomizer = register_module('EventRandomizer')
# Root output module
output = register_module('RootOutput')
output.param('outputFileName', outputName)
# Show progress of processing
progress = register_module('Progress')

# ============================================================================
# Randomize events

main = create_path()
main.add_module(progress)
main.add_module(rootinput)
main.add_module(randomizer)
#
main.add_module(output)

outpath = create_path()

main.add_path(outpath)

# Set branching condition
randomizer.if_value('>0', outpath)

# print to check
print main

# Process events
process(main)

# Print call statistics
print statistics
#
