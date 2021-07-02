#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2

# skip the event in the output if it doesn't contain trg data
skim_dummy_trg = True
# whether to save the output dst file
save_output = False
# use an input file that is already unpacked
from_unpacked = False

input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = b2.register_module('SeqRootInput')
else:
    root_input = b2.register_module('RootInput')

main = b2.create_path()
main.add_module(root_input)

histo = b2.register_module('HistoManager')
histoutput_name = input_files.front().split('/')[-1]
histoutput_name = './monitor_plots/' + histoutput_name[:histoutput_name.rfind('.')] + '.unpacked.root'
histo.param("histoFileName", histoutput_name)
main.add_module(histo)


if not from_unpacked:
    main.add_module('Progress')
    unpacker = b2.register_module('CDCTriggerUnpacker')
    unpacker.logging.log_level = b2.LogLevel.DEBUG
    # increase this value to get debug mesages in more detail
    unpacker.logging.debug_level = 10
    unpacker.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)
    # size (number of words) of the Belle2Link header
    unpacker.param('headerSize', 3)
    # unpack the data from the 2D tracker and save its Bitstream
    unpacker.param('unpackTracker2D', True)
    # make CDCTriggerSegmentHit objects from the 2D input
    unpacker.param('decode2DFinderInput', True)
    unpacker.param('decode2DFinderTrack', True)
    # it seems the B2L for 2D0 and 2D1 are swapped
    unpacker.param('2DNodeId', [
        [0x11000001, 0],
        [0x11000001, 1],
        [0x11000002, 0],
        [0x11000002, 1]])

    main.add_module(unpacker)

    if skim_dummy_trg:
        # skip if there are no trigger data in the event
        empty_path = b2.create_path()
        unpacker.if_false(empty_path)

    main.add_module('TRGCDCT2DDQM')


if save_output:
    # save the output root file with specified file name
    output_name = input_files.front().split('/')[-1]
    output_name = output_name[:output_name.rfind('.')] + '.unpacked.root'
    main.add_module('RootOutput',
                    outputFileName=output_name,
                    excludeBranchNames=['RawCDCs',
                                        'RawECLs',
                                        'RawKLMs',
                                        'RawSVDs',
                                        'RawPXDs',
                                        'RawTOPs'])

b2.process(main)
print(b2.statistics)
