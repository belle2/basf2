##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2


def add_neuro_2d_unpackers(path, debug_level=4, debugout=True):
    unpacker = b2.register_module('CDCTriggerUnpacker')
    if debugout:
        unpacker.logging.log_level = b2.LogLevel.DEBUG
        # increase this value to get debug mesages in more detail
        unpacker.logging.debug_level = debug_level
        unpacker.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)
    # size (number of words) of the Belle2Link header
    unpacker.param('headerSize', 3)
    # unpack the data from the 2D tracker and save its Bitstream
    unpacker.param('unpackTracker2D', True)
    # make CDCTriggerTrack and CDCTriggerSegmentHit objects from the 2D output
    unpacker.param('decode2DFinderTrack', True)
    # make CDCTriggerSegmentHit objects from the 2D input
    unpacker.param('decode2DFinderInput', True)
    unpacker.param('2DNodeId', [
        [0, 0],
        [0x11000001, 0],
        [0, 0],
        [0, 0]])
    unpacker.param('NeuroNodeId', [
        [0x11000001, 1],
        [0, 0],
        [0, 0],
        [0, 0]])

    unpacker.param('unpackNeuro', True)
    unpacker.param('decodeNeuro', True)
    path.add_module(unpacker)


b2.set_log_level(b2.LogLevel.ERROR)
b2.use_database_chain()
b2.use_central_database("data_reprocessing_prod6")

main = b2.create_path()
dstfiles = ['/hsm/belle2/bdata/Data/\
release-02-01-00/DB00000438/prod00000006/e0003/4S/\
r05613/all/dst/sub00/dst.physics.0003.05613.HLT2.f00000.root']
main.add_module("RootInput", inputFileNames=dstfiles)
add_neuro_2d_unpackers(main, debug_level=2, debugout=False)

main.add_module('HistoManager',
                histoFileName='histogram.root')

main.add_module('CDCTriggerDQM')
main.add_module('Progress')

b2.process(main)
print(b2.statistics)
