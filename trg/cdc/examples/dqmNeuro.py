from basf2 import *
from ROOT import Belle2
import sys
import os

print('begin')


def add_neuro_2d_unpackers(path, debug_level=4, debugout=True):
    unpacker = register_module('CDCTriggerUnpacker')
    if debugout:
        unpacker.logging.log_level = LogLevel.DEBUG
        # increase this value to get debug mesages in more detail
        unpacker.logging.debug_level = debug_level
        unpacker.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    # size (number of words) of the Belle2Link header
    unpacker.param('headerSize', 3)
    # unpack the data from the 2D tracker and save its Bitstream
    unpacker.param('unpackTracker2D', True)
    # make CDCTriggerTrack and CDCTriggerSegmentHit objects from the 2D output
    unpacker.param('decode2DFinderTrack', True)
    # make CDCTriggerSegmentHit objects from the 2D input
    unpacker.param('decode2DFinderInput', True)
    unpacker.param('2DNodeId', [
        [0x11000001, 0],
        [0x11000001, 1],
        [0x11000002, 0],
        [0x11000002, 1],
    ])
    # TODO: load Finesse IDs of active boards in the current run from the database
    unpacker.param('NeuroNodeId', [
        [0x11000005, 0],
        [0x11000005, 1],
        [0x11000006, 0],
        [0x11000006, 1],
    ])

    unpacker.param('unpackNeuro', True)
    unpacker.param('decodeNeuro', True)
    path.add_module(unpacker)


def add_neuro_simulation_swts(path):
    path.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v2.2.coe"),
                    OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v2.2.coe"),
                    TSHitCollectionName="CDCTriggerSegmentHitsSW")
    path.add_module('CDCTrigger2DFinder',
                    minHits=4, minHitsShort=4, minPt=0.3,
                    hitCollectionName="CDCTriggerSegmentHitsSW",
                    outputCollectionName="TRGCDC2DFinderTracksSWTS")
    path.add_module('CDCTriggerNeuro',
                    inputCollectionName='TRGCDC2DFinderTracksSWTS',
                    outputCollectionName='TRGCDCNeuroTracksSWTSSW2D',
                    hitCollectionName='CDCTriggerSegmentHitsSW',
                    filename=Belle2.FileSystem.findFile("data/trg/cdc/Background2.0_20161207.root"),
                    writeMLPinput=True,
                    fixedPoint=True,
                    alwaysTrackT0=True,
                    )


def add_neuro_simulation(path):
    path.add_module('CDCTriggerNeuro',
                    inputCollectionName='CDCTriggerNNInput2DFinderTracks',
                    outputCollectionName='TSimNeuroTracks',
                    hitCollectionName='CDCTriggerNNInputSegmentHits',
                    # TODO: load used network from the database
                    filename=Belle2.FileSystem.findFile("data/trg/cdc/Background2.0_20161207.root"),
                    # 'CosmicsBfield_20171006.root'
                    # 'FixedAbsPt350MeV_20170112_Bkg.root'
                    writeMLPinput=True,
                    fixedPoint=True,
                    alwaysTrackT0=True,
                    )


set_log_level(LogLevel.ERROR)
use_central_database("data_reprocessing_prompt")

main = create_path()

dstfiles = [sys.argv[1]]
outputfile = ''
dstputfile = ''
os.makedirs('dqmoutput/data', exist_ok=True)
os.makedirs('dqmoutput/hist', exist_ok=True)
os.makedirs('dqmoutput/log', exist_ok=True)
print('somewhere', sys.argv[1])
if '.sroot' in sys.argv[1]:
    outputfile = 'dqmoutput/hist/histo.' + sys.argv[1].split('/')[-1].split('.sroot')[0] + '.root'
    dstputfile = 'dqmoutput/data/dst.' + sys.argv[1].split('/')[-1].split('.sroot')[0] + '.root'
    main.add_module("SeqRootInput", inputFileNames=dstfiles)
elif '.root' in sys.argv[1]:
    outputfile = 'dqmoutput/hist/histo.' + sys.argv[1].split('/')[-1].split('.root')[0] + '.root'
    dstputfile = 'dqmoutput/data/dst.' + sys.argv[1].split('/')[-1].split('.root')[0] + '.root'
    main.add_module("RootInput", inputFileNames=dstfiles)

main.add_module('Gearbox')
main.add_module('Geometry')

main.add_module('Progress', maxN=3)
add_neuro_2d_unpackers(main, debug_level=2, debugout=False)
add_neuro_simulation(main)
add_neuro_simulation_swts(main)
showRecoTracks = 'yes'
if showRecoTracks == 'yes':
    main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName='TSimNeuroTracks',
                    hitCollectionName='CDCTriggerNNInputSegmentHits', axialOnly=True)
    main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName='CDCTriggerNeuroTracks',
                    hitCollectionName='CDCTriggerNNInputSegmentHits', axialOnly=True)
    main.add_module(
        'CDCTriggerRecoMatcher',
        TrgTrackCollectionName='TRGCDCNeuroTracksSWTSSW2D',
        hitCollectionName='CDCTriggerSegmentHitsSW',
        axialOnly=True)
    main.add_module('SetupGenfitExtrapolation')

main.add_module('HistoManager',
                histoFileName=outputfile)
main.add_module('CDCTriggerDQM',
                simNeuroTracksName='TSimNeuroTracks',
                showRecoTracks=showRecoTracks,
                simNeuroTracksSWTSSW2DName='TRGCDCNeuroTracksSWTSSW2D')
main.add_module('RootOutput', outputFileName=dstputfile)

process(main)
print(statistics)
