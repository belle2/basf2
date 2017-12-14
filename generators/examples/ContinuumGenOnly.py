#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 100 continuum events using EvtGen (using EvtGen, i.e. without ISR)
# using the custom BELLE2_DECAY file.
#
# Example steering file
########################################################

from basf2 import *
from generators import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)

main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# run
main.add_module("Progress")
main.add_module("Gearbox")

# use default continuum production
add_continuum_generator(main, finalstate='ccbar')

# add full root output
main.add_module("RootOutput", outputFileName="continuum.root")

# print MC particles (for debugging)
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
