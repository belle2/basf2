import basf2
from ROOT import Belle2

################################################################################
# Defining some standard names for trigger tracks here:
################################################################################

# standard name for hardware tracks coming from the unpacker
hwneurotracks = 'CDCTriggerNeuroTracks'
hwneuroinput2dfindertracks = 'CDCTriggerNNInput2DFinderTracks'
hwneuroinputsegmenthits = 'CDCTriggerNNInputSegmentHits'


# sofware neurotracks, that were calculated from hardware 2d-inputtracks
# and hardware stereo track segments coming over all 48cc within one event
# OR
# standard name for tracks resulting from the software neurotrigger
# module when it is rerun on the hardware tracks
hwsimneurotracks = 'TSimNeuroTracks'

# software neurotracks, which were all simulated in software starting only
# from real CDCHITs. Also simulated track segments and 2dtracks are needed therefore.
simneurotracks_swtssw2d = 'TRGCDCNeuroTracks'
simsegmenthits = 'SimSegmentHits'
sim2dtracks_swts = 'TRGCDC2DFinderTracks'

################################################################################
# Defining a filter for skipping trigger suppressed events in realdata:
################################################################################


class filterTRG(basf2.Module):
    def initialize(self, branchname=hwneuroinput2dfindertracks):
        self.branchname = branchname
        self.nullpath = basf2.create_path()

    def event(self):
        self.return_value(bool(Belle2.PyStoreArray(self.branchname).getEntries() > 0))
        self.if_false(self.nullpath)


def add_neuro_unpacker(path, debug_level=4, debugout=False, **kwargs):
    #
    unpacker = basf2.register_module('CDCTriggerUnpacker')
    if debugout:
        unpacker.logging.log_level = LogLevel.DEBUG
        unpacker.logging.debug_level = debug_level
        unpacker.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    # size (number of words) of the Belle2Link header
    unpacker.param('headerSize', 3)
    # unpack the data from the 2D tracker and save its Bitstream
    unpacker.param('unpackTracker2D', False)
    # make CDCTriggerTrack and CDCTriggerSegmentHit objects from the 2D output
    unpacker.param('decode2DFinderTrack', False)
    # make CDCTriggerSegmentHit objects from the 2D input
    unpacker.param('decode2DFinderInput', False)
    unpacker.param('NeuroNodeId', [
        [0x11000005, 0],
        [0x11000005, 1],
        [0x11000006, 0],
        [0x11000006, 1],
    ])
    if 'useDB' in kwargs:
        unpacker.param('useDB', kwargs['useDB'])
    else:
        unpacker.param('useDB', True)

    if 'sim13dt' in kwargs:
        unpacker.param('sim13dt', kwargs['sim13dt'])
    else:
        unpacker.param('sim13dt', False)
    unpacker.param('unpackNeuro', True)
    unpacker.param('decodeNeuro', True)
    path.add_module(unpacker)


def add_neuro_2d_unpackers(path, debug_level=4, debugout=False, **kwargs):
    #
    unpacker = basf2.register_module('CDCTriggerUnpacker')
    if debugout:
        unpacker.logging.log_level = LogLevel.DEBUG
        unpacker.logging.debug_level = debug_level
        unpacker.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    # size (number of words) of the Belle2Link header
    unpacker.param('headerSize', 3)
    # unpack the data from the 2D tracker and save its Bitstream
    unpacker.param('unpackTracker2D', False)
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
    unpacker.param('NeuroNodeId', [
        [0x11000005, 0],
        [0x11000005, 1],
        [0x11000006, 0],
        [0x11000006, 1],
    ])
    if 'useDB' in kwargs:
        unpacker.param('useDB', kwargs['useDB'])
    else:
        unpacker.param('useDB', True)

    if 'sim13dt' in kwargs:
        unpacker.param('sim13dt', kwargs['sim13dt'])
    else:
        unpacker.param('sim13dt', False)
    unpacker.param('unpackNeuro', True)
    unpacker.param('decodeNeuro', True)
    path.add_module(unpacker)


def add_neurotrigger_sim(path, nntweightfile=None, debug_level=4, debugout=False, **kwargs):
    nnt = basf2.register_module('CDCTriggerNeuro')
    if 'inputCollectionName' in kwargs:
        nnt.param('inputCollectionName', kwargs['inputCollectionName'])
    else:
        nnt.param('inputCollectionName', hwneuroinput2dfindertracks)
    if 'outputCollectionName' in kwargs:
        nnt.param('outputCollectionName', kwargs['outputCollectionName'])
    else:
        nnt.param('outputCollectionName', hwsimneurotracks)
    if 'hitCollectionName' in kwargs:
        nnt.param('hitCollectionName', kwargs['hitCollectionName'])
    else:
        nnt.param('hitCollectionName', hwneuroinputsegmenthits)
    if 'writeMLPinput' in kwargs:
        nnt.param('writeMLPinput', kwargs['writeMLPinput'])
    else:
        nnt.param('writeMLPinput', True)
    if 'fixedPoint' in kwargs:
        nnt.param('fixedPoint', kwargs['fixedPoint'])
    else:
        nnt.param('fixedPoint', True)
    if nntweightfile is not None:
        nnt.param('filename', Belle2.FileSystem.findFile(nntweightfile))

    if 'et_option' in kwargs:
        nnt.param('et_option', kwargs['et_option'])
    if debugout:
        nnt.logging.log_level = LogLevel.DEBUG
        nnt.logging.debug_level = debug_level
    path.add_module(nnt)


def add_neurotrigger_hw(path, nntweightfile=None, debug_level=4, debugout=False, **kwargs):
    nnt = basf2.register_module('CDCTriggerNeuro')
    if 'inputCollectionName' in kwargs:
        nnt.param('inputCollectionName', kwargs['inputCollectionName'])
    else:
        nnt.param('inputCollectionName', hwneurotracks)
    if 'outputCollectionName' in kwargs:
        nnt.param('outputCollectionName', kwargs['outputCollectionName'])
    else:
        nnt.param('outputCollectionName', hwsimneurotracks)
    if 'hitCollectionName' in kwargs:
        nnt.param('hitCollectionName', kwargs['hitCollectionName'])
    else:
        nnt.param('hitCollectionName', hwneuroinputsegmenthits)
    if 'writeMLPinput' in kwargs:
        nnt.param('writeMLPinput', kwargs['writeMLPinput'])
    else:
        nnt.param('writeMLPinput', True)
    if 'fixedPoint' in kwargs:
        nnt.param('fixedPoint', kwargs['fixedPoint'])
    else:
        nnt.param('fixedPoint', True)
    if 'realinputCollectonName' in kwargs:
        nnt.param('realinputCollectionName', kwargs['realinputCollectionName'])
    else:
        nnt.param('realinputCollectionName', hwneuroinput2dfindertracks)

    if nntweightfile is not None:
        nnt.param('filename', Belle2.FileSystem.findFile(nntweightfile))
    nnt.param('NeuroHWTrackInputMode', True)
    if 'et_option' in kwargs:
        nnt.param('et_option', kwargs['et_option'])
    if debugout:
        nnt.logging.log_level = LogLevel.DEBUG
        nnt.logging.debug_level = debug_level
    path.add_module(nnt)


def add_neuro_simulation(path):
    path.add_module('CDCTriggerTSF',
                    InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v2.2.coe"),
                    OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v2.2.coe"),
                    TSHitCollectionName=simsegmenthits)
    path.add_module('CDCTrigger2DFinder',
                    minHits=4, minHitsShort=4, minPt=0.3,
                    hitCollectionName=simsegmenthits,
                    outputCollectionName=sim2dtracks_swts)
    path.add_module('CDCTriggerNeuro',
                    inputCollectionName=sim2dtracks_swts,
                    outputCollectionName=simneurotracks_swtssw2d,
                    hitCollectionName=simsegmenthits,
                    writeMLPinput=True,
                    fixedPoint=True,
                    )
