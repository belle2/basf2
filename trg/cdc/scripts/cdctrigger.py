# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


def add_cdc_trigger(path, SimulationMode=1, minHits=4):
    """
    This function adds the CDC trigger modules to a path.
    @path              modules are added to this path
    @SimulationMode    the simulation mode in TSIM, 1: fast simulation,
                       trigger algoritm simulation only, no firmware simulation
                       2: full simulation, both trigger algorithm and firmware
                       are simulated
    @minHits           the minimum number of  super layers with hits, the default
                       values is 4
    """
    if SimulationMode == 1:
        # TSF
        path.add_module('CDCTriggerTSF',
                        InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
                        OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"))
        # 2D finder
        path.add_module('CDCTriggerHoughtracking',
                        outputCollectionName='TRG2DFinderTracks',
                        minHits=minHits)
        # ETF
        path.add_module('CDCTriggerETF')
        # fitters
        path.add_module('CDCTrigger2DFitter',
                        inputCollectionName='TRG2DFinderTracks',
                        outputCollectionName='TRG2DFitterTracks')
        path.add_module('CDCTrigger3DFitter',
                        inputCollectionName='TRG2DFitterTracks',
                        outputCollectionName='TRG3DFitterTracks')
        # neurotrigger
        path.add_module('NeuroTrigger',
                        filename=Belle2.FileSystem.findFile("data/trg/cdc/Neuro20170109.root"),
                        inputCollectionName='TRG2DFinderTracks',
                        outputCollectionName='TRGNNTracks')
    elif SimulationMode == 2:
        # standard CDC trigger
        trgcdc = register_module('TRGCDC')
        trgcdc_params = {
            'ConfigFile': Belle2.FileSystem.findFile("data/trg/cdc/TRGCDCConfig_0_20101111.dat"),
            'InnerTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_v3.0.coe"),
            'OuterTSLUTFile': Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_v3.0.coe"),
            'HoughFinderMappingFileMinus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingMinus20160223.dat"),
            'HoughFinderMappingFilePlus': Belle2.FileSystem.findFile("data/trg/cdc/HoughMappingPlus20160223.dat"),
            'SimulationMode': SimulationMode,
            '2DfinderCollection': 'TRGOld2DFinderTracks',
            '2DfitterCollection': 'TRG2DFitterTracks',
            '3DfitterCollection': 'TRG3DFitterTracks'}
        trgcdc.param(trgcdc_params)
        path.add_module(trgcdc)
