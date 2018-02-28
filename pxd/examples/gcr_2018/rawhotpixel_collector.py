#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import argparse
from basf2 import *
from ROOT import Belle2

set_log_level(LogLevel.INFO)

gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# Select Beast2_phase2 geometry
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')

# Select to build only PXD s
geometry.param('components', ['PXD'])

main = create_path()
main.add_module('RootInput', branchNames=['PXDRawHits'])
main.add_module('HistoManager', histoFileName='RawCollectorOutput.root')
main.add_module(gearbox)
main.add_module(geometry)
main.add_module('PXDRawHotPixelMaskCollector', granularity="all")
main.add_module('Progress')
process(main)

print(statistics)
