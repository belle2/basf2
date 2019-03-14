#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Usage:
# basf2 rawdata_simulation.py -o outputfilename.sroot


import basf2

from simulation import add_simulation
from softwaretrigger import constants

from rawdata import add_packers

# You could use your own components here or just use the default for the HLT (everything except PXD)
# e.g. without SVD
# components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
# if you leave out the components in all calls, the default will be used
components = constants.DEFAULT_HLT_COMPONENTS

main_path = basf2.create_path()

main_path.add_module("EventInfoSetter", evtNumList=[10])
main_path.add_module("EvtGenInput")

add_simulation(main_path, components)
add_packers(main_path, components=components)

main_path.add_module("SeqRootOutput",
                     outputFileName="hlt_rawdata_simulation.sroot",
                     saveObjs=["EventMetaData"] + constants.RAWDATA_OBJECTS)

basf2.print_path(main_path)
basf2.process(main_path)

print(basf2.statistics)
