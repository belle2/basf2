#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Run PYTHIA on unfragmented final states
#
# Example steering file
########################################################

from basf2 import *
from ROOT import Belle2

# suppress messages and warnings during processing:
set_log_level(LogLevel.RESULT)

# event info setter
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# use root file with e.g. uubar pairs from KKMC  as input
rootinput = register_module('RootInput')
rootinput.param('inputFileName', './kkmc_uu.root')

# get the gearbox
gearbox = register_module('Gearbox')

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

# add fragmentation module
fragmentation = register_module('Fragmentation')
fragmentation.param('ParameterFile', Belle2.FileSystem.findFile('data/generators/modules/fragmentation/pythia_belle2.dat'))
fragmentation.param('ListPYTHIAEvent', 0)
fragmentation.param('UseEvtGen', 1)
fragmentation.param('UserDecFile', Belle2.FileSystem.findFile('data/generators/modules/fragmentation/dec_belle2_qqbar.dec'))

# add root output module
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', './fragmentation.root')

# main
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(rootinput)
main.add_module(fragmentation)

# branch to an empty path if PYTHIA failed
emptypath = create_path()
fragmentation.if_value('<1', emptypath)

main.add_module(rootoutput)
# main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)
process(main)
