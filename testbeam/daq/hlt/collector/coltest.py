#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *
from simulation import register_simulation
from reconstruction import register_reconstruction


set_log_level(LogLevel.ERROR)

argvs = sys.argv
argc = len(argvs)

# print argvs[1]
# print argc

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

# register sim/recon modules
register_simulation(components)
register_reconstruction(components)

# create a main path
main = create_path()

# Add input module
input = register_module("SeqRootInput")
# input.param ( "inputFileName", "/fcdisk1-1/data/sim/sim-evtgen.sroot")
input.param("inputFileName", "/pcidisk1-1/data/rec/rec-evtgen.sroot")
main.add_module(input)

# create geometry
gearbox = register_module('Gearbox')
main.add_module(gearbox)
geometry = register_module('Geometry')
main.add_module(geometry)

# Add output module
output = register_module("SeqRootOutput")
output.param("outputFileName", "/dev/null")

# Add Rbuf2Ds
# rbuf2ds = register_module("Rbuf2Ds")
# rbuf2ds.param("RingBufferName", argvs[1])
# main.add_module(rbuf2ds)

# Add Progress
progress = register_module("Progress")
main.add_module(progress)

# Add Ds2Rbuf
# ds2rbuf = register_module("Ds2Rbuf")
# ds2rbuf.param("RingBufferName", argvs[2])
# main.add_module(ds2rbuf)

# Run
process(main, 5000)
