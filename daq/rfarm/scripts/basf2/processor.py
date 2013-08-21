#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# command arguments
#   argvs[1] = input ring buffer name
#   argvs[2] = output ring buffer name
#   argvs[3] = number of cores for parallel processing
argvs = sys.argv
argc = len(argvs)

#set_log_level(LogLevel.INFO)
set_log_level(LogLevel.ERROR)

# detecor components to be reconstructed
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'ECL',
    ]

# create a main path
main = create_path()

# Input
#input = register_module('SeqRootInput')
#input.param('inputFileName', '/fcdisk1-1/data/sim/sim-evtgen.sroot' )
#main.add_module(input)

# Rbuf2Ds as input module
rbuf2ds = register_module("Rbuf2Ds")
rbuf2ds.param("RingBufferName", argvs[1])
main.add_module(rbuf2ds)

# create geometry
gearbox = register_module('Gearbox')
main.add_module(gearbox)
geometry = register_module('Geometry')
main.add_module(geometry)

# Reconstruction
add_reconstruction(main, components)

# Output
#output = register_module('SeqRootOutput')
#output.param('outputFileName', '/pcidisk1-1/data/rec/rec-evtgen.sroot' )
#main.add_module(output)

# Ds2Rbuf as output module
ds2rbuf = register_module("Ds2Rbuf")
ds2rbuf.param("RingBufferName", argvs[2])
main.add_module(ds2rbuf)

# Show progress
progress = register_module('Progress')
main.add_module(progress)
elapsed = register_module('ElapsedTime')
main.add_module(elapsed)

# Start processing
nprocess(int(argvs[3]))
process(main)

# Print call statistics
print statistics
