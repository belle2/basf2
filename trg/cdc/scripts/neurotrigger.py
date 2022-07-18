##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from ROOT import Belle2
import nntd

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


def filterTRG(path):
    nullpath = basf2.create_path()
    mfilter = basf2.register_module("CDCTriggerUnpacker", unpackNeuro=True)
    path.add_module(mfilter)
    mfilter.if_value('<1', nullpath)


class nnt_eventfilter(basf2.Module):
    def initialize(self,
                   tracksegmentsname=hwneuroinputsegmenthits,
                   twodtracksname=hwneuroinput2dfindertracks,
                   neurotracksname=hwneurotracks,
                   recotracksname="RecoTracks"
                   ):
        self.tracksegmentsname = tracksegmentsname
        self.twodtracksname = twodtracksname
        self.neurotracksname = neurotracksname
        self.recotracksname = recotracksname
        self.nullpath = basf2.create_path()

    def event(self):
        self.return_value(bool(self.neurotrack_allgoodquality()))
        self.if_false(self.nullpath)

    def hastrginfo(self):
        return bool(Belle2.PyStoreArray(self.twodtracksname).getEntries() > 0)

    def neurotrack_allgoodquality(self):
        isgoodquality = True
        for tr in Belle2.PyStoreArray("CDCTriggerNeuroTracks"):
            if tr.getQualityVector() > 0:
                isgoodquality = False
                break
        return isgoodquality


class randommaker(basf2.Module):
    def initialize(self):
        self.counter = 0

    def event(self):
        print("counter is " + str(self.counter))
        if self.counter % 1000 == 0:
            print("case 0")
            self.return_value(0)
        elif self.counter % 3 == 0:
            print("case 1")
            self.return_value(1)
        elif self.counter % 3 == 1:
            print("case 2")
            self.return_value(2)
        elif self.counter % 3 == 2:
            print("case 3")
            self.return_value(3)
        else:
            print("some kind of error")
        self.counter += 1


def add_train_output(path, baseOutputFileName, baseAnaFileName, excludeBranchNames=[], branchNames=[]):
    # create 4 output paths:
    outpaths = []
    for x in range(4):
        outpaths.append([".random_"+str(x), basf2.create_path()])
    # add the randommaker module:
    rm = basf2.register_module(randommaker())
    path.add_module(rm)

    # also add nnta module:

    rm.if_value('==0', outpaths[0][1])
    rm.if_value('==1', outpaths[1][1])
    rm.if_value('==2', outpaths[2][1])
    rm.if_value('==3', outpaths[3][1])
    for p in outpaths:
        nnta = basf2.register_module(nntd.nntd())
        nnta.param({"filename": baseAnaFileName+p[0]+".pkl", "netname": "default_all"})
        p[1].add_module("RootOutput", outputFileName=baseOutputFileName +
                        p[0]+".root", excludeBranchNames=excludeBranchNames, branchNames=branchNames)
        p[1].add_module(nnta)


def add_neuro_unpacker(path, debug_level=4, debugout=False, **kwargs):
    #
    unpacker = basf2.register_module('CDCTriggerUnpacker')
    if debugout:
        unpacker.logging.log_level = basf2.LogLevel.DEBUG
        unpacker.logging.debug_level = debug_level
        unpacker.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)
    # size (number of words) of the Belle2Link header
    unpacker.param('headerSize', 3)
    # unpack the data from the 2D tracker and save its Bitstream
    unpacker.param('unpackTracker2D', False)
    # make CDCTriggerTrack and CDCTriggerSegmentHit objects from the 2D output
    unpacker.param('decode2DFinderTrack', False)
    # make CDCTriggerSegmentHit objects from the 2D input
    unpacker.param('decode2DFinderInput', False)
    unpacker.param('2DNodeId', [
        [0x11000001, 0],
        [0x11000001, 1],
        [0x11000002, 0],
        [0x11000002, 1]])
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
        unpacker.logging.log_level = basf2.LogLevel.DEBUG
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
    if 'EventTimeName' in kwargs:
        nnt.param('EventTimeName', kwargs['EventTimeName'])
    if debugout:
        nnt.logging.log_level = basf2.LogLevel.DEBUG
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
    else:
        nnt.param('et_option', 'etf_or_fastestpriority')

    if 'EventTimeName' in kwargs:
        nnt.param('EventTimeName', kwargs['EventTimeName'])
    else:
        nnt.param('EventTimeName', 'CDCTriggerNeuroETFT0')
    if debugout:
        nnt.logging.log_level = basf2.LogLevel.DEBUG
        nnt.logging.debug_level = debug_level
    path.add_module(nnt)


def add_neuro_simulation(path, nntweightfile=None, **kwargs):
    nnt = basf2.register_module('CDCTriggerNeuro')
    tsf = basf2.register_module('CDCTriggerTSF')
    if "InnerTSLUTFile" in kwargs:
        tsf.param("InnerTSLUTFile", kwargs["InnerTSLUTFile"])
    else:
        tsf.param("InnerTSLUTFile", Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v2.2.coe"))
    if "OuterTSLUTFile" in kwargs:
        tsf.param("OuterTSLUTFile", kwargs["OuterTSLUTFile"])
    else:
        tsf.param("OuterTSLUTFile", Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v2.2.coe"))
    tsf.param("TSHitCollectionName", simsegmenthits)
    tsf.param("CDCHitCollectionName", "CDCHits")
    if "makeRecoLRTable" in kwargs:
        tsf.param("makeRecoLRTable", kwargs["makeRecoLRTable"])
    if "makeTrueLRTable" in kwargs:
        tsf.param("makeTrueLRTable", kwargs["makeTrueLRTable"])
    path.add_module(tsf)
    path.add_module('CDCTrigger2DFinder',
                    minHits=4, minHitsShort=4, minPt=0.3,
                    hitCollectionName=simsegmenthits,
                    outputCollectionName=sim2dtracks_swts)
    if nntweightfile is not None:
        nnt.param('filename', Belle2.FileSystem.findFile(nntweightfile))
    nnt.param('inputCollectionName', sim2dtracks_swts)
    nnt.param('outputCollectionName', simneurotracks_swtssw2d)
    nnt.param('hitCollectionName', simsegmenthits)
    nnt.param('writeMLPinput', True)
    nnt.param('fixedPoint', True)
    path.add_module(nnt)
