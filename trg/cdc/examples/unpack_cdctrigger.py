#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################
# example steering file to unpack the CDC Trigger data from B2L
###############################################################

# usage: basf2 SOME_PATH/unpack_cdctrigger.py -i inputFile.[s]root [-o outputFile.root]
# (items in the square brackets [] are optional)

from basf2 import *
from ROOT import Belle2

# whether we will also unpack CDC data
unpack_CDCHit = True
# don't save the event in the output if it doesn't contain trg data
skim_dummy_trg = True

input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = register_module('SeqRootInput')
else:
    root_input = register_module('RootInput')

main = create_path()
main.add_module(root_input)

if unpack_CDCHit:
    # Set Database
    use_database_chain()
    use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
    cdc_unpacker = register_module('CDCUnpacker')
    cdc_unpacker.param('enableStoreCDCRawHit', True)
    main.add_module(cdc_unpacker)

unpacker = register_module('CDCTriggerUnpacker')
unpacker.logging.log_level = LogLevel.DEBUG
# increase this value to get debug mesages in more detail
unpacker.logging.debug_level = 10
unpacker.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
# size (number of words) of the Belle2Link header
unpacker.param('headerSize', 3)
# unpack the data from the 2D tracker and save its Bitstream
unpacker.param('unpackTracker2D', True)
# make CDCTriggerTrack and CDCTriggerSegmentHit objects from the 2D output
unpacker.param('decode2DFinderTrack', True)
# make CDCTriggerSegmentHit objects from the 2D input
unpacker.param('decode2DFinderInput', True)
# it seems the B2L for 2D0 and 2D1 are swapped
unpacker.param('2DNodeId', [
    [0x11000001, 1],
    [0x11000001, 0],
    [0x11000002, 0],
    [0x11000002, 1]])

main.add_module(unpacker)

if skim_dummy_trg:
    # don't save the output if there are no trigger data in the event
    empty_path = create_path()
    unpacker.if_false(empty_path)

# save the output root file with specified file name
main.add_module('RootOutput',
                outputFileName='unpackedCDCTrigger.root',
                excludeBranchNames=['RawCDCs',
                                    'RawECLs',
                                    'RawKLMs',
                                    'RawSVDs',
                                    'RawPXDs',
                                    'RawTOPs'])
process(main)
print(statistics)
