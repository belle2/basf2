#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 100 continuum events using EvtGen (using EvtGen, i.e. without ISR)
# using the custom BELLE2_DECAY file.
#
# Example steering file
########################################################

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.INFO)

main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# to run the framework the used modules need to be registered
evtgen = register_module('EvtGenInput')
evtgen.param('ParentParticle', 'vpho')
evtgen.param('userDECFile', os.environ['BELLE2_LOCAL_DIR'] + '/generators/evtgen/decayfiles/ccbar+Dst.dec')

# run
main.add_module("Progress")
main.add_module("Gearbox")
main.add_module(evtgen)
main.add_module("RootOutput", outputFileName="evtgen_continuum.root")
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
