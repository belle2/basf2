from basf2 import *
from ROOT import Belle2
import sys
import os


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
                    et_option='fastestpriority',
                    )


def add_neuro_simulation(path):
    path.add_module('CDCTriggerNeuro',
                    inputCollectionName='CDCTrigger2DFinderTracks',
                    outputCollectionName='TSimNeuroTracks',
                    hitCollectionName='CDCTriggerSegmentHits',
                    # TODO: load used network from the database
                    filename=Belle2.FileSystem.findFile("data/trg/cdc/Background2.0_20161207.root"),
                    # 'CosmicsBfield_20171006.root'
                    # 'FixedAbsPt350MeV_20170112_Bkg.root'
                    writeMLPinput=True,
                    fixedPoint=True,
                    et_option='fastestpriority',
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


add_neuro_simulation(main)
add_neuro_simulation_swts(main)
showRecoTracks = True
if showRecoTracks:
    main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName='TSimNeuroTracks',
                    hitCollectionName='CDCTriggerSegmentHits', axialOnly=True)
    main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName='CDCTriggerNeuroTracks',
                    hitCollectionName='CDCTriggerSegmentHits', axialOnly=True)
    main.add_module(
        'CDCTriggerRecoMatcher',
        TrgTrackCollectionName='TRGCDCNeuroTracksSWTSSW2D',
        hitCollectionName='CDCTriggerSegmentHitsSW',
        axialOnly=True)
    main.add_module('SetupGenfitExtrapolation')

main.add_module('HistoManager',
                histoFileName=outputfile)
main.add_module('CDCTriggerNeuroDQM',
                simNeuroTracksName='TSimNeuroTracks',
                simNeuroTracksSWTSSW2DName='TRGCDCNeuroTracksSWTSSW2D',
                unpackedNeuroInput2dTracksName='CDCTrigger2DFinderTracks',
                unpackedNeuroInputSegmentHits='CDCTriggerSegmentHits',
                showRecoTracks=showRecoTracks,
                recoTrackMultiplicity=1,
                skipWithoutHWTS=False,
                maxRecoZDist=3.0
                )
main.add_module('RootOutput', outputFileName=dstputfile)

process(main)
print(statistics)
