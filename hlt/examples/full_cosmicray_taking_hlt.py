#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Usage:
# basf2 run_hlt.py -i inputfilename.sroot -o outputfilename.sroot
# you can take the output file of rawdata_simulation.py as the input file

import basf2

from simulation import add_simulation
import os

from reconstruction import add_cosmics_reconstruction
from softwaretrigger.path_functions import add_unpackers

components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
saverawdataObjs = ['EventMetaData', 'RawCDCs', 'RawTOPs', 'RawARICHs', 'RawECLs', 'RawKLMs']

main_path = basf2.create_path()

main_path.add_module("SeqRootInput")

add_unpackers(main_path, components)

add_cosmics_reconstruction(main_path, components)

output = basf2.register_module('SeqRootOutput')
output.param('saveObjs', saverawdataObjs)
main_path.add_module(output)

basf2.print_path(main_path)
basf2.process(main_path)

print(basf2.statistics)
