#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from rawdata import add_unpackers
from reconstruction import add_reconstruction, add_mdst_output
from softwaretrigger.constants import SoftwareTriggerModes
from softwaretrigger.processing import add_hlt_processing

# create path
main = create_path()

# input
main.add_module('RootInput')

# gearbox and geometry
main.add_module('Gearbox')
main.add_module('Geometry')

# reconstruction
# No filter is applied in monitoring mode. To enable filtering, use, for example,
# softwaretrigger_mode=SoftwareTriggerModes.filter
add_hlt_processing(main, softwaretrigger_mode=SoftwareTriggerModes.monitor, prune_output=False, calcROIs=False)

# mdst output
add_mdst_output(main)

# process events and print call statistics
process(main)
print(statistics)
