#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from ROOT import Belle2
from monitor_module import Monitor

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

if not from_unpacked:
    main.add_module('Progress')
    # unpack CDC data
    # Set Database
    b2.use_database_chain()
    b2.use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
    cdc_unpacker = b2.register_module('CDCUnpacker')
    cdc_unpacker.param('enableStoreCDCRawHit', True)
    main.add_module(cdc_unpacker)

    unpacker = b2.register_module('CDCTriggerUnpacker')
    unpacker.logging.log_level = b2.LogLevel.DEBUG
    # increase this value to get debug messages in more detail
    unpacker.logging.debug_level = 10
    unpacker.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)
    # size (number of words) of the Belle2Link header
    unpacker.param('headerSize', 3)
    # unpack the data from the 2D tracker and save its Bitstream
    unpacker.param('unpackTracker2D', True)
    # make CDCTriggerSegmentHit objects from the 2D input
    unpacker.param('decode2DFinderInput', True)
    # it seems the B2L for 2D0 and 2D1 are swapped
    unpacker.param('2DNodeId', [
        [0x11000001, 1],
        [0x11000001, 0],
        [0x11000002, 0],
        [0x11000002, 1]])
    unpacker.param('2DNodeId_pcie40', [
        [0x10000001, 0],
        [0x10000001, 1],
        [0x10000001, 2],
        [0x10000001, 3]])

    main.add_module(unpacker)

    if skim_dummy_trg:
        # skip if there are no trigger data in the event
        empty_path = b2.create_path()
        unpacker.if_false(empty_path)

    main.add_module('Gearbox')
    main.add_module('Geometry', components=['BeamPipe',
                                            'PXD', 'SVD', 'CDC',
                                            'MagneticFieldConstant4LimitedRCDC'])
    cdcdigitizer = b2.register_module('CDCDigitizer')
    # ...CDCDigitizer...
    # set digitizer to no smearing
    param_cdcdigi = {'Fraction': 1,
                     'Resolution1': 0.,
                     'Resolution2': 0.,
                     'Threshold': -10.0}
    cdcdigitizer.param(param_cdcdigi)
    cdcdigitizer.param('AddInWirePropagationDelay', True)
    cdcdigitizer.param('AddTimeOfFlight', True)
    cdcdigitizer.param('UseSimpleDigitization', True)
    main.add_module(cdcdigitizer)
    main.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                    OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"),
                    TSHitCollectionName='TSimSegmentHits')
else:
    save_output = False

main.add_module(Monitor())

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
