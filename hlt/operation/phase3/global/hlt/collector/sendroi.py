#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

import basf2 as b2
# from simulation import register_simulation
# from reconstruction import register_reconstruction

b2.set_log_level(b2.LogLevel.ERROR)

argvs = sys.argv
argc = len(argvs)

# print argvs[1]
# print argc
# argvs[1] = input ring buffer
# argvs[2] = input ring buffer
# argvs[3] = RoI queue name

print("MQname = " + argvs[3])

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

# Local database access
databasefile = "/dev/shm/LocalDB/database.txt"
b2.prepend_testing_payloads(databasefile)

# Register modules to declare objects
# register_simulation(components)
# register_reconstruction(components)

# Register RoI related modules for object decoding
# roiGen = register_module('ROIGenerator')
# roiPayloadAssembler = register_module('ROIPayloadAssembler')

# to avoid undefined symbol
geom = b2.register_module("Geometry")

# create a main path
main = b2.create_path()

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
main.add_module(rbuf2ds)

# RoI related codes here
# roiReadTest = register_module('ROIReadTest')
# param_roiReadTest = {'outfileName': 'ROIoutReceiver.txt',
#                     'ROIpayloadName': 'ROIpayload'}
# roiReadTest.param(param_roiReadTest)
# main.add_module(roiReadTest)

# RoI sender
roisender = b2.register_module('ROISender')
roisender.param("MessageQueueName", argvs[3])
roisender.param("MessageQueueDepth", 20)
roisender.param("MessageSize", 16384)
roisender.param("ROIpayloadName", "ROIpayload")
main.add_module(roisender)

# Add Progress
progress = b2.register_module("Progress")
progress.param('maxN', 4)
main.add_module(progress)

# Add Elapsed Time
etime = b2.register_module("ElapsedTime")
etime.param("EventInterval", 20000)
main.add_module(etime)

# Add Ds2Raw
ds2raw = b2.register_module("Ds2Raw")
ds2raw.param("RingBufferName", argvs[2])
main.add_module(ds2raw)


# Run
b2.process(main)
