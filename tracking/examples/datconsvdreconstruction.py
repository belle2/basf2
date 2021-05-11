#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run the reconstruction on Phase2 geometry
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from L1trigger import add_tsim

# background (collision) files
bg = None

b2.set_random_seed(3)

num_events = 100

output_filename = "datconsvdreconstruction.root"

output_branches = ["DATCONSVDDigits", "DATCONSimpleSVDCluster", "DATCONSVDSpacePoints"]

# create path
main = b2.create_path()

# specify number of events to be generated
# the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=num_events)

# in case you need to fix seed of random numbers
# set_random_seed('some fixed value')

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main, bkgfiles=bg, bkgOverlay=True)

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)

main.add_module('SVDShaperDigitToDATCONSVDDigitConverter')

main.add_module('DATCONSVDSimpleClusterizer')

main.add_module('DATCONSVDSpacePointCreator')

main.add_module('Progress')

# Finally add output
main.add_module("RootOutput", outputFileName=output_filename, branchNames=output_branches)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
