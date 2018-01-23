#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# This file can serve as a basis for the sterring files
# deployed on the HLT trigger nodes. Simply the Event Simulation
# must be removed and replaced by SeqRoot input and output
#

import basf2

from simulation import add_simulation
from rawdata import add_unpackers, add_packers

from softwaretrigger.path_functions import (
    setup_softwaretrigger_database_access,
    add_softwaretrigger_reconstruction,
    add_softwaretrigger_dqm,
    DEFAULT_HLT_COMPONENTS
)

setup_softwaretrigger_database_access()

main_path = basf2.create_path()

components = DEFAULT_HLT_COMPONENTS

# generate Y4s event as test input to the reconstruction and filter chain

# SIMULATED HLT INPUT STARTS HERE
main_path.add_module("EventInfoSetter", evtNumList=[10])
main_path.add_module("EvtGenInput")

add_simulation(main_path, components=components)
add_packers(main_path, components=components)
# SIMULATED HLT INPUT ENDS HERE

# HLT Processing starts here

# add the unpackers required on HLT
add_unpackers(main_path, components=components)
# add HLT reconstruction and software trigger filter
# remove 'store_array_debug_prescale' parameter, if you want the software trigger
# to actually filter events and not write out their RAW objects
add_softwaretrigger_reconstruction(main_path, store_array_debug_prescale=1, components=components)
# HLT Processing ends here

add_softwaretrigger_dqm(main_path)
main_path.add_module("HistoManager", histoFileName="hlt_reco_histomanager.root")

main_path.add_module("RootOutput", outputFileName="output.root")

basf2.print_path(main_path)
basf2.process(main_path)

print(basf2.statistics)
