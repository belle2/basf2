#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run the reconstruction on Phase2 geometry
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from datcon.datcon_functions import add_datcon

b2.set_random_seed(1337)

num_events = 10

output_filename = "datconPXDDataReduction.root"

additionalBranchNames = ["DATCONRecoTracks", "DATCONPXDIntercepts", "DATCONROIs"]

# create path
main = b2.create_path()

main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=num_events)

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation, don't perform data reduction per default, but use DATCON instead
add_simulation(main, bkgOverlay=True, forceSetPXDDataReduction=True, usePXDDataReduction=False, cleanupPXDDataReduction=False)

# reconstruction
add_reconstruction(main)

# add DATCON simulation using the optimized DATCON
add_datcon(main)

main.add_module('Progress')

# Finally add output
main.add_module("RootOutput", outputFileName=output_filename, additionalBranchNames=additionalBranchNames)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
