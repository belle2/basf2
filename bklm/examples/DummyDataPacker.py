#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
argvs = sys.argv

if len(argvs) < 3:
    print('Usage : DummyDataPacker.py <# of events> <node ID> <output filename>')
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Reader
max_event = int(argvs[1])
nodeid = int(argvs[2])
packer = register_module('BKLMRawPacker')
packer.param('MaxEventNum', max_event)
packer.param('NodeID', nodeid)
# output = register_module('RootOutput')
# output.param('outputFileName', 'muForBKLM.root')
gearbox = register_module('Gearbox')

# Geometry builder
geobuilder = register_module('Geometry')
geobuilder.log_level = LogLevel.INFO
geobuilder.param('components', ['BKLM'])

bklmUnpack = register_module('BKLMUnpacker')
# BKLM reco
bklmreco = register_module('BKLMReconstructor')
bklmreco.log_level = LogLevel.INFO

# efficiencies...
bklmEff = register_module('BKLMEffnRadio')
# File output
# dump = register_module('SeqRootOutput')
# dump = register_module('RootOutput')
# dump.param('outputFileName', argvs[3])

# Create main path
main = create_path()

# Add modules to main path
main.add_module(packer)
main.add_module(gearbox)
main.add_module(geobuilder)
main.add_module(bklmUnpack)
main.add_module(bklmreco)
main.add_module(bklmEff)
# main.add_module(output)
# main.add_module(dump)

# Process all events
process(main)
