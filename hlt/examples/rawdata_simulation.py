#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Usage:
# basf2 rawdata_simulation.py -o outputfilename.sroot


import basf2

from simulation import add_simulation
import os

from rawdata import add_packers

from softwaretrigger.path_functions import (
    setup_softwaretrigger_database_access,
    RAWDATA_OBJECTS,
    DEFAULT_HLT_COMPONENTS,
)

# Create a path to generate some raw-data samples and then use the software trigger path(s) to reconstruct them.
setup_softwaretrigger_database_access()

# You could use your own components here or just use the default for the HLT (everything except PXD)
# e.g. without SVD
# components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
# if you leave out the components in all calls, the default will be used
components = DEFAULT_HLT_COMPONENTS

main_path = basf2.create_path()

main_path.add_module("EventInfoSetter", evtNumList=[10])
main_path.add_module("EvtGenInput")

add_simulation(main_path, components)
add_packers(main_path, components=components)

main_path.add_module("SeqRootOutput",
                     outputFileName="hlt_rawdata_simulation.sroot",
                     saveObjs=["EventMetaData"] + RAWDATA_OBJECTS)

basf2.print_path(main_path)
basf2.process(main_path)

print(basf2.statistics)
