#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run TREPS to generate e+e- -> e+e-pi+pi- events
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
parameterFilePipi = Belle2.FileSystem.findFile('generators/modules/trepsinput/parameterFiles/treps_par_pipi.dat')
wListTableFilePipi = Belle2.FileSystem.findFile('generators/modules/trepsinput/wListFiles/wlist_table_pipi.dat')

trepsinput = basf2.register_module('trepsinput')
trepsinput.param('ParameterFile', parameterFilePipi)
trepsinput.param('WListTableFile', wListTableFilePipi)
trepsinput.param('UseDiscreteAndSortedW', True)

# run
main.add_module("Progress")
main.add_module(trepsinput)
main.add_module("RootOutput", outputFileName="utrepsbpipi_100k_dicrete.root")
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.INFO, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
