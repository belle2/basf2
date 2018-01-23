#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Usage:
# the script running on hlt for cosmic

import basf2
import os
from reconstruction import add_cosmics_reconstruction
from rawdata import add_unpackers
from softwaretrigger import (
    add_fast_reco_software_trigger,
    add_hlt_software_trigger,
    add_calibration_software_trigger,
)
from softwaretrigger.hltdqm import cosmic_hltdqm


components = ["CDC", "ECL", "TOP", "ARICH", "BKLM", "EKLM"]
saverawdataObjs = ['EventMetaData', 'RawCDCs', 'RawTOPs', 'RawARICHs', 'RawECLs', 'RawKLMs', 'SoftwareTriggerResult']

main_path = basf2.create_path()

main_path.add_module("SeqRootInput")

histoman = basf2.register_module('HistoManager')
main_path.add_module(histoman)

add_unpackers(main_path, components=components)

# cosmic reconstruction
add_cosmics_reconstruction(main_path, components=components)
# hlt trigger modules for test
add_fast_reco_software_trigger(main_path)
add_hlt_software_trigger(main_path)
add_calibration_software_trigger(main_path)

cosmic_hltdqm(main_path)

output = basf2.register_module('SeqRootOutput')
output.param('saveObjs', saverawdataObjs)
main_path.add_module(output)

basf2.print_path(main_path)
basf2.process(main_path)

print(basf2.statistics)
