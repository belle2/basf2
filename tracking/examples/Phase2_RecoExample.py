#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run the reconstruction on Phase2 geometry
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

# background (collision) files
# bg = glob.glob('./BG/*.root')
# on KEKCC: (choose one of the sets)
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase2/set*/*.root
bg = None

# number of events to generate, can be overriden with -n
num_events = 100
# output filename, can be overriden with -o
output_filename = "RootOutput_Phase2.root"

# create path
main = b2.create_path()

# specify number of events to be generated
# the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
main.add_module("EventInfoSetter", expList=1002, runList=1, evtNumList=num_events)

# in case you need to fix seed of random numbers
# set_random_seed('some fixed value')

# generate BBbar events
main.add_module('EvtGenInput')

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# Finally add output
main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
