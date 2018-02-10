#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 100 generic BBbar events using EvtGen
#
# Example steering file
########################################################

from basf2 import *
from generators import *

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

# EvtGen
add_evtgen_generator(path=main, finalstate='mixed')

# run
main.add_module("Progress")
main.add_module("Gearbox")
main.add_module("RootOutput", outputFileName="evtgen_upsilon4s.root")
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
