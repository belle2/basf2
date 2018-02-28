#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################
#
# Generate 100 events using KKMC+PYTHIA from generators/scripts
#
# Contributor(s): Torben Ferber (torben.ferber@desy.de)
#
###############################################################

from basf2 import *
from generators import *

# suppress messages and warnings during processing
set_log_level(LogLevel.INFO)

main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# run
main.add_module("Progress")

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
