#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run the SVD Unpacker on a downloaded
# raw data .root file.
# It decodes a RawSVD object (from COPPER) and produce a list of SVDDigit
# according to the xml map of the sensors.
# The user has to supply the path to his input raw data file and to output
# root file with SVDDigits.
#
##############################################################################

from basf2 import *

# show warnings during processing
set_log_level(LogLevel.WARNING)

# Register modules

input = register_module('RootInput')
input.param('inputFileName', '/path/to/input/rawdatafile.root')
input.param('branchNames', ['RawSVDs'])

progress = register_module('Progress')

svdUnpacker = register_module('SVDUnpacker')
svdUnpacker.param('rawSVDListName', 'RawSVDs')
svdUnpacker.param('svdDigitListName', 'SVDDigits')
svdUnpacker.param('xmlMapFileName',\
    'testbeam/vxd/data/SVD-OnlineOfflineMap.xml')

output = register_module('RootOutput')
output.param('outputFileName', '/path/to/unpacked/datafile.root')
output.param('excludeBranchNames', ['RawSVDs'])

# ============================================================================
# Do the simulation

main = create_path()
main.add_module(input)
main.add_module(progress)
main.add_module(svdUnpacker)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
#
