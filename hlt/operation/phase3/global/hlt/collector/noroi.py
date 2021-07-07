#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys

import basf2 as b2
# from simulation import register_simulation
# from reconstruction import register_reconstruction

b2.set_log_level(b2.LogLevel.ERROR)

argvs = sys.argv
argc = len(argvs)

print(argvs[1])
print(argc)

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

# Register modules to declare objects
# register_simulation(components)
# register_reconstruction(components)

# Register RoI related modules for object decoding
roiGen = b2.register_module('ROIGenerator')
roiPayloadAssembler = b2.register_module('ROIPayloadAssembler')

# create a main path
main = b2.create_path()

# Load Geometry module
gearbox = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')

# Add input module
# input = register_module("SeqRootInput")
# input.param ( "inputFileName", "/fcdisk1-1/data/sim/sim-evtgen.sroot")
# main.add_module(input)

# Add output module
# output= register_module("SeqRootOutput")
# output.param ( "outputFileName", "/dev/null" )
# main.add_module(output)

# Add Rbuf2Ds
rbuf2ds = b2.register_module("Rbuf2Ds")
# rbuf2ds = register_module("FastRbuf2Ds")
rbuf2ds.param("RingBufferName", argvs[1])
# rbuf2ds.param("NumThreads", 4 )
main.add_module(rbuf2ds)

# Add Progress
progress = b2.register_module("Progress")
progress.param('maxN', 4)
main.add_module(progress)

# Add Elapsed Time
elapsed = b2.register_module("ElapsedTime")
elapsed.param("EventInterval", 20000)
main.add_module(elapsed)

# Add Ds2Rbuf
# ds2rbuf = register_module("Ds2Rbuf")
# ds2rbuf.param("RingBufferName", argvs[2])
# main.add_module(ds2rbuf)

# Add Ds2Raw
ds2rbuf = b2.register_module("Ds2Raw")
ds2rbuf.param("RingBufferName", argvs[2])
main.add_module(ds2rbuf)

# Test seqrootoutput
# output = register_module("SeqRootOutput" )
# output.param ( "outputFileName", "/dev/null" )
# main.add_module(output)

# Run
b2.process(main)
