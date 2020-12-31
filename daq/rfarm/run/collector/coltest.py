#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

import basf2 as b2
from simulation import register_simulation
from reconstruction import register_reconstruction


b2.set_log_level(b2.LogLevel.ERROR)

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
main = b2.create_path()

# Add input module
input = b2.register_module("SeqRootInput")
# input.param ( "inputFileName", "/fcdisk1-1/data/sim/sim-evtgen.sroot")
input.param("inputFileName", "/pcidisk1-1/data/rec/rec-evtgen.sroot")
main.add_module(input)

# create geometry
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# Add output module
output = b2.register_module("SeqRootOutput")
output.param("outputFileName", "/dev/null")

# Add Rbuf2Ds
# rbuf2ds = register_module("Rbuf2Ds")
# rbuf2ds.param("RingBufferName", argvs[1])
# main.add_module(rbuf2ds)

# Add Progress
progress = b2.register_module("Progress")
main.add_module(progress)

# Add Ds2Rbuf
# ds2rbuf = register_module("Ds2Rbuf")
# ds2rbuf.param("RingBufferName", argvs[2])
# main.add_module(ds2rbuf)

# Run
b2.process(main, 5000)
