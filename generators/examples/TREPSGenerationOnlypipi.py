#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run KKMC to generate tautau events
#
# Example steering file
########################################################

import basf2
from basf2 import set_log_level, LogLevel, process, statistics
from ROOT import Belle2
from beamparameters import add_beamparameters

set_log_level(LogLevel.DEBUG)

# main path
main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=99966)

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)

# to run the framework the used modules need to be registered
trepsinput = basf2.register_module('trepsinput')
# trepsinput.param('W', 2.0)
trepsinput.param('InputFileName', './treps_par.dat')
trepsinput.param('RootFileNameForCheck', './treps_test.rot')

# run
main.add_module("Progress")
main.add_module(trepsinput)
main.add_module("RootOutput", outputFileName="utrepsbpipi_100k_B.root")
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.INFO, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
