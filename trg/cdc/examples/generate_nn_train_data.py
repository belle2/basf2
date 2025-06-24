import basf2
from ROOT import Belle2
from reconstruction import add_reconstruction
import rawdata

basf2.conditions.append_globaltag('patch_main_release-09')

nthreads = 30
RecoTrackName = "RecoTracks"
TrackFinderCollectionName = "Track2D_2"
TrackSegmentCollectionName = "TrackSegment"
TrackNeuroCollectionName = "TrackNN"
EventTimeCollectionName = "CDCTRGEventTimeETF"
TRGSummaryCollectionName = "TRGSummary"


def add_cdc_unpacker(path, debug_level=4, debugout=False, **kwargs):
    #
    cdc_unpacker = basf2.register_module('CDCUnpacker')
    cdc_unpacker.param('enableStoreCDCRawHit', True)
    path.add_module(cdc_unpacker)

    unpacker = basf2.register_module('CDCTriggerUnpacker')
    unpacker.logging.log_level = basf2.LogLevel.DEBUG
    # increase this value to get debug mesages in more detail
    unpacker.logging.debug_level = debug_level
    unpacker.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)
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
    unpacker.param('2DNodeId_pcie40', [
        [0x10000001, 0],
        [0x10000001, 1],
        [0x10000001, 2],
        [0x10000001, 3]])

    path.add_module(unpacker)


main = basf2.create_path()

input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = basf2.register_module('SeqRootInput')
else:
    root_input = basf2.register_module('RootInput')

main.add_module(root_input)
# loading gearbox and geometry, which is needed for simulation: ###
main.add_module('Gearbox')
main.add_module('Geometry')

# add filter to skip events without neurotrigger b2link information
# neurotrigger.filterTRG(main)
#
main.add_module('SetupGenfitExtrapolation')
# show progress at least every 10^maxN events: ###
main.add_module('Progress', maxN=3)
# main.add_module('EventLimiter',maxEventsPerRun=100000)

# add unpacker function from the rawdata script or the neurotrigger script: ###
rawdata.add_unpackers(main)
add_cdc_unpacker(main)

# add reconstruction in case .sroot files were used: ###
add_reconstruction(main, add_trigger_calculation=False)

main.add_module('CDCTriggerTSF',
                CDCHitCollectionName="CDCHits",
                InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v2.2.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v2.2.coe"),
                TSHitCollectionName=TrackSegmentCollectionName,
                SaveADC=True,  # save adc for NN train data generation
                ADC_cut_threshold=20)

main.add_module('CDCTrigger2DFinder',
                minPt=0.3,
                hitCollectionName=TrackSegmentCollectionName,
                outputCollectionName=TrackFinderCollectionName)

main.add_module('CDCTriggerRecoMatcher', TrgTrackCollectionName=TrackFinderCollectionName,
                hitCollectionName=TrackSegmentCollectionName, axialOnly=True)

main.add_module('CDCTriggerHoughETF',
                t0CalcMethod=2,
                useHighPassTimingList=False,
                usePriorityTiming=False,
                storeTracks=True,
                hitCollectionName=TrackSegmentCollectionName,
                outputCollectionName='CDCTriggerETFTracks',
                outputEventTimeName=EventTimeCollectionName,
                offset=-10)

main.add_module('CDCTriggerNeuroData',
                hitCollectionName=TrackSegmentCollectionName,
                inputCollectionName=TrackFinderCollectionName,
                trainOnRecoTracks=True,
                targetCollectionName='RecoTracks',
                EventTimeName=EventTimeCollectionName,
                NeuroTrackInputMode=False,
                singleUse=True,
                writeconfigFileName='',
                configFileName=Belle2.FileSystem.findFile('trg/cdc/examples/test_config_for_generate_nn_train_data.conf'),
                SaveFakeTrack=True,
                logLevel=basf2.LogLevel.DEBUG,  # show some debug output
                debugLevel=500,
                gzipFilename='out2.gz')

basf2.process(main)
