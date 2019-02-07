#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to run the framework
# for different experiment, run and event numbers.
#
# In the example below, basf2 will run on and display
# the following experiment, run and event numbers:
#
# Experiment 71, Run  3, 4 Events
# Experiment 71, Run  4, 6 Events
# Experiment 73, Run 10, 2 Events
# Experiment 73, Run 20, 5 Events
# Experiment 73, Run 30, 3 Events
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *
import sys
argv = sys.argv

# Set the log level to show only error and fatal messages
# set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# gearbox = register_module('Gearbox')
# SVDUnpack = register_module('SVDUnpacker')
# SVDClust = register_module('SVDClusterizer')
# vxdtf = register_module('VXDTF')
# SVD_DQM = register_module('SVDDQM')
# vxdtf_dqm = register_module('VXDTFDQM')
# trackfitter = register_module('GenFitter')
# roiprod = register_module('PXDDataReduction')
# roipayload = register_module('ROIPayloadAssembler')

# Modules
deserializer = register_module('StorageDeserializer')
deserializer.param('InputBufferName', argv[1])
deserializer.param('InputBufferSize', int(argv[2]))
if len(argv) >= 6:
    deserializer.param('NodeName', argv[5])
    deserializer.param('NodeID', int(argv[6]))
    deserializer.param('UseShmFlag', int(argv[7]))
    deserializer.param('EB2', int(argv[8]))
output = register_module('StorageSerializer')
output.param('OutputBufferName', argv[3])
output.param('OutputBufferSize', int(argv[4]))
output.param('NodeID', int(argv[6]))
output.param('compressionLevel', 0)

# Create main path
main = create_path()

# Add modules to main path
main.add_module(deserializer)
main.add_module(output)

# Process all events
process(main)
