#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from reconstruction import add_reconstruction

main = create_path()

# input
input = register_module('RootInput')
input.param('inputFileName', 'B2Kpi_gen.root')
main.add_module(input)

# detecor simulation
# include or exclude detector components you need
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    ]

# Load XML files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Build geometry for track fitting etc...
geometry = register_module('Geometry')
geometry.param('components', components)
main.add_module(geometry)

# reconstruction
add_reconstruction(main, components)
# or add_reconstruction(main) to run the reconstruction of all detectors

output = register_module('RootOutput')
output.param('outputFileName', 'B2Kpi_rec.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics
