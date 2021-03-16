# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2


def add_cdc_trigger(path, SimulationMode=1, shortTracks=False, lowPt=False,
                    thetaDef='avg', zDef='min'):
    """
    This function adds the CDC trigger modules to a path.
    @path              modules are added to this path
    @SimulationMode    the simulation mode in TSIM, 1: fast simulation,
                       trigger algoritm simulation only, no firmware simulation
                       2: full simulation, both trigger algorithm and firmware
                       are simulated
    @shortTracks       the standard track finding requires hits in 4 axial super layers.
                       with the shortTracks option, tracks with hits in the innermost
                       3 super layers are also found.
    @lowPt             default threshold for track finding is 0.3Gev.
                       with the lowPt option, the threshold is 0.255GeV.
                       use together with the shortTracks option.
    @thetaDef          theta definition for the CDCTriggerTrackCombiner
    @zDef              z definition for the CDCTriggerTrackCombiner
                       (for details see module description)
    """

    if SimulationMode == 1:
        # TSF
        path.add_module('CDCTriggerTSF',
                        InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                        OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"))
        # 2D finder
        if shortTracks:
            minHitsShort = 3
        else:
            minHitsShort = 4
        if lowPt:
            minPt = 0.255
        else:
            minPt = 0.3
        path.add_module('CDCTrigger2DFinder',
                        minHits=4, minHitsShort=minHitsShort, minPt=minPt)

        # Old ETF
        # path.add_module('CDCTriggerETF', trueEventTime=trueEventTime)
        # ETF priority fastest timings among 2D Tracks
        path.add_module('CDCTriggerHoughETF')

        # fitters
        path.add_module('CDCTrigger2DFitter')
        path.add_module('CDCTrigger3DFitter')
        # neurotrigger
        if shortTracks:
            b2.B2ERROR("shortTracks=True is deprecated and no longer supported! "
                       "Network weights will now be loaded from the database. "
                       "If you really want to use shorttracks, load the specific network "
                       "weights in the Neurotrigger module!")
            exit()
        path.add_module('CDCTriggerNeuro')

        path.add_module('CDCTriggerTrackCombiner',
                        thetaDefinition=thetaDef, zDefinition=zDef)
    elif SimulationMode == 2:
        b2.B2WARNING("full simulation mode does not include all CDC trigger modules yet")
        # standard CDC trigger
        trgcdc = b2.register_module('TRGCDC')
        trgcdc_params = {
            'ConfigFile': Belle2.FileSystem.findFile("data/trg/cdc/TRGCDCConfig_0_20101111.dat"),
            'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
            'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
            'HoughFinderMappingFileMinus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingMinus20160223.dat"),
            'HoughFinderMappingFilePlus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingPlus20160223.dat"),
            'SimulationMode': SimulationMode,
            '2DfinderCollection': 'TRGCDC2DFinderTracks',
            '2DfitterCollection': 'TRGCDC2DFitterTracks',
            '3DfitterCollection': 'TRGCDC3DFitterTracks'}
        trgcdc.param(trgcdc_params)
        path.add_module(trgcdc)
