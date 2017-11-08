#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run the reconstruction on Phase2 geometry
#############################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output
from L1trigger import add_tsim
from ROOT import Belle2
import glob


# background (collision) files
# put here the directory for phase2 bkg MC
# bg = glob.glob('/group/belle2/users/jbennett/BG15th/phase2/set0/*.root') # if you run at KEKCC
# bg = glob.glob('./BG/*.root')
bg = None

# number of events to generate, can be overriden with -n
num_events = 100
# output filename, can be overriden with -o
output_filename = "RootOutput_Phase2.root"

# create path
main = create_path()

# the sectormaps for Phase2 are currently only in the development tag
# Also note that these SectorMaps are preliminary and may change
use_central_database("development", loglevel=LogLevel.INFO)

# specify number of events to be generated
# the experiment number has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
main.add_module("EventInfoSetter", expList=1002, runList=1, evtNumList=num_events)

# generate BBbar events
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# Gearbox to override the default geometry with the one for Phase 2
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
main.add_module(gearbox)


# detector simulation
add_simulation(main, bkgfiles=bg)

# remove the cache for background files to reduce memory
if bg is not None:
    set_module_parameters(main, "BeamBkgMixer", cacheSize=0)

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)

# Finally add output
main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
process(main)
print(statistics)
