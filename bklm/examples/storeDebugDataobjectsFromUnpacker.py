#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
#
#  This steering file shows how to correctly store
#  the dataobjects created from the raw data by the
#  BKLMUnpacker module:
#    - BKLMDigit
#    - BKLMEventDigitDebug
#    - BKLMDigitOutOfRange
#
#  The relationship between these dataobjects are
#  automatically stored by RootOutput.
#
#
#  Contact: Giacomo De Pietro (2018)
#           giacomo.depietro@roma3.infn.it
#
########################################################

from basf2 import *
from rawdata import add_unpackers

components = ['BKLM']

# Set the input and output file names
inputName = '/ghi/fs01/belle2/bdata/Data/Raw/e0003/r04794/sub00/physics.0003.04794.HLT2.f00000.root'
outputName = 'dataobjects_e0003_r04794_HLT2_f00000.root'

use_central_database("development", LogLevel.WARNING)

# Add the modules for the input files
input = register_module('RootInput')
input.param('inputFileNames', [inputName])

# Add the Gearbox and the Geometry
# Not needed, since we only unpack raw data
# gearbox = register_module('Gearbox')
# geometry = register_module('Geometry')
# geometry.param('components', components)
# geometry.param('useDB', False)

# Add the BKLMUnpacker
# We use the default script for unpackers, see later
# unpacker = register_module('BKLMUnpacker')
# unpacker.param('keepEvenPackages', 1)
# unpacker.param('SciThreshold', 0)
# unpacker.param("useDefaultModuleId", 1)
# unpacker.param('loadMapFromDB', 0)
# unpacker.param('rawdata', 1)

# Add RootOutput
output = register_module('RootOutput')
output.param('outputFileName', outputName)
output.param('branchNames', ['BKLMDigits', 'BKLMDigitOutOfRanges', 'BKLMEventDigitDebugs'])
output.param('branchNamesPersistent', 'FileMetaData')

# Create the main path
main = create_path()

# Add modules to the path
main.add_module(input)
main.add_module('Progress')
# Add default script for unpackers
add_unpackers(main, components=components)
main.add_module(output)

# Process only 10k events
process(main, 10000)
print(statistics)
