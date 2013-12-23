#!/bin/bash


export MCGEN_HOME=$PWD
source /sw/belle2/tools/setup_belle2.sh
cd /sw/belle2/releases/build-2013-02-24/ #Replace with newer build or your own
setuprel
cd $MCGEN_HOME


basf2 <<EOF >& logfile_reconstruction.log
#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

main = create_path()

# input
input = register_module('RootInput')
input.param('inputFileName', 'output.root')
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
              #              'ARICH',
              #              'BKLM',
              #              'ECL',
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
output.param('outputFileName', 'output_reconstructed.root')
main.add_module(output)

process(main)

# Print call statistics
print statistics

EOF
