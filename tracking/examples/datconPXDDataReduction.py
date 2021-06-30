#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Steering file to provide an example on how to use DATCON
# for data reduction in the PXD
#############################################################

import basf2 as b2
from simulation import add_simulation
from tracking import add_hit_preparation_modules
from datcon.datcon_functions import add_datcon

b2.set_random_seed(1337)

num_events = 10

output_filename = "datconPXDDataReduction.root"

# create path
main = b2.create_path()

main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=num_events)

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation, don't perform data reduction per default, but use DATCON instead
add_simulation(main, bkgOverlay=False, forceSetPXDDataReduction=True, usePXDDataReduction=False, cleanupPXDDataReduction=False)

# add DATCON simulation using the DATCON
add_hit_preparation_modules(main, components=["SVD"])
add_datcon(main)

main.add_module('Progress')

# Finally add output
main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
