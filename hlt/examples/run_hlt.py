#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Usage:
# basf2 run_hlt.py -i inputfilename.sroot -o outputfilename.sroot
# you can take the output file of rawdata_simulation.py as the input file

import basf2

from simulation import add_simulation
import os

from rawdata import add_raw_seqoutput, add_unpackers

from softwaretrigger.path_functions import (
    setup_softwaretrigger_database_access,
    add_softwaretrigger_reconstruction,
    DEFAULT_HLT_COMPONENTS,
)

# Create a path to generate some raw-data samples and then use the software trigger path(s) to reconstruct them.
# setup_softwaretrigger_database_access()

# You could use your own components here or just use the default for the HLT (everything except PXD)
# e.g. without SVD
#     components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
# if you leave out the components in all calls, the default will be used
components = DEFAULT_HLT_COMPONENTS

main_path = basf2.create_path()

main_path.add_module("SeqRootInput", inputFileName="hlt_rawdata_simulation.sroot")

# adding the HistoManager is important because the add_softwaretrigger_reconstruction call
# adds DQM modules which will crash, if the HistoManager is not registered
main_path.add_module("HistoManager")

add_unpackers(main_path, components=components)
add_softwaretrigger_reconstruction(main_path, store_array_debug_prescale=1, components=components)

add_raw_seqoutput(main_path, additionalObjects=["SoftwareTriggerResults", "SoftwareTriggerVariables", "ROIs"])

basf2.print_path(main_path)
basf2.process(main_path)

print(basf2.statistics)
