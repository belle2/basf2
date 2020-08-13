#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################
#
# Generate 100 events using KKMC+PYTHIA from generators/scripts
#
# Contributor(s): Torben Ferber (torben.ferber@desy.de)
#
###############################################################

import basf2
from generators import add_continuum_generator

# suppress messages and warnings during processing
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# run
main.add_module("Progress")

# use default continuum production
add_continuum_generator(main, finalstate='ccbar')

# add full root output
main.add_module("RootOutput", outputFileName="continuum.root")

# print MC particles (for debugging)
main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
