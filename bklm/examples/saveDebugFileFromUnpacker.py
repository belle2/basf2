#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
#
#  This steering file shows how to correctly enable
#  the production of the debug file from the
#  BKLMUnpacker module.
#
#  The debug file is useful to quickly check the sanity
#  of the BKLM (dead sectors, dead layers, etc.)
#  and can be used also to analyze local runs.
#
#  The input file from the example is from a local run.
#
#  In order to analyze the debug file:
#  > root -b drawHitmap.C
#
#  Contact: Giacomo De Pietro (2018)
#           giacomo.depietro@roma3.infn.it
#
########################################################

from basf2 import *

# Set the input and output file names
inputName = '/home/belle2/giacomo/ghi/BKLM/root_output-181009-ut3trigger-2KHz-1trg2ms.sroot'
outputName = 'bklm_ut3_20181009.root'

use_central_database("development", LogLevel.WARNING)

# Add the modules for the input files
input = register_module('SeqRootInput')
# input = register_module('RootInput')
input.param('inputFileNames', [inputName])

# Set the Gearbox and the Geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('components', ['BKLM'])

# Add the unpacker
unpacker = register_module('BKLMUnpacker')
unpacker.param('keepEvenPackages', 1)
unpacker.param('SciThreshold', 0)
unpacker.param("useDefaultModuleId", 1)
unpacker.param('loadMapFromDB', 0)
unpacker.param('rawdata', 1)
# And enable the creation of the debug file
unpacker.param('enableDebugFile', 1)
unpacker.param('nameDebugFile', outputName)

# Create the main path
main = create_path()

# Add modules to the path
main.add_module(input)
main.add_module('Progress')
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(unpacker)

# Process only 10k events
process(main, 10000)
print(statistics)
