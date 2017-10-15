#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from basf2 import *
from ROOT import Belle2
from cdctrigger import add_cdc_trigger

"""
generate tracks with particle gun, simulate CDC and CDC trigger, save the output.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 10000
evtnum = 100
particlegun_params = {
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'inversePt',
    'momentumParams': [0.3, 10.],
    'thetaGeneration': 'uniform',
    'thetaParams': [35, 145],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0, 0.0],
    'yVertexParams': [0, 0.0],
    'zVertexParams': [-50.0, 50.0]}

# ------------------------- #
# create path up to trigger #
# ------------------------- #

# set random seed
basf2.set_random_seed(seed)
# suppress messages and warnings during processing:
# basf2.set_log_level(basf2.LogLevel.ERROR)

main = basf2.create_path()

main.add_module('EventInfoSetter', evtNumList=evtnum)
main.add_module('Progress')
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe',
                                        'PXD', 'SVD', 'CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
particlegun = basf2.register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)
main.add_module('FullSim')
main.add_module('CDCDigitizer')

# ---------------------- #
# CDC trigger and output #
# ---------------------- #

SimulationMode = 1
minHits = 4
trueEventTime = False
# TSF
main.add_module('CDCTriggerTSF',
                InnerTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/innerLUT_Bkg_p0.70_b0.80.coe"),
                OuterTSLUTFile=Belle2.FileSystem.findFile("data/trg/cdc/outerLUT_Bkg_p0.70_b0.80.coe"))
# 2D finder
original2d = register_module('CDCTrigger2DFinder')
# original2d.logging.log_level = basf2.LogLevel.DEBUG
# original2d.logging.debug_level = 20
# original2d.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)
original2d.param('testFilename', 'tracks.txt')

fast2d = register_module('CDCTrigger2DFinder')
# fast2d.logging.log_level = basf2.LogLevel.DEBUG
# fast2d.logging.debug_level = 20
# fast2d.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)
fast2d.param('testFilename', 'fasttracks.txt')
fast2d.param('suppressClone', True)
fast2d.param('outputCollectionName', "TRGCDC2DFinderFastTracks")
main.add_module(original2d)
main.add_module(fast2d)


# ETF
main.add_module('CDCTriggerETF', trueEventTime=trueEventTime)
# fitters
main.add_module('CDCTrigger2DFitter')
main.add_module('CDCTrigger2DFitter',
                inputCollectionName="TRGCDC2DFinderFastTracks",
                outputCollectionName="TRGCDC2DFitterFastTracks")
main.add_module('CDCTrigger3DFitter')
main.add_module('CDCTrigger3DFitter',
                inputCollectionName="TRGCDC2DFitterFastTracks",
                outputCollectionName="TRGCDC3DFitterFastTracks")
# neurotrigger
main.add_module('CDCTriggerNeuro',
                filename=Belle2.FileSystem.findFile("data/trg/cdc/Neuro20170405_LUTBkg.root"))
main.add_module('CDCTriggerNeuro',
                filename=Belle2.FileSystem.findFile("data/trg/cdc/Neuro20170405_LUTBkg.root"),
                inputCollectionName="TRGCDC2DFinderFastTracks",
                outputCollectionName="TRGCDCNeuroFastTracks")


class Ana(Module):
    """analyze the difference between 2D with and without clone suppression"""

    def initialize(self):
        self.event_info = Belle2.PyStoreObj('EventMetaData')
        self.mc = Belle2.PyStoreArray('MCParticles')
        self.trk2d = Belle2.PyStoreArray('TRGCDC2DFinderTracks')
        self.fast_trk2d = Belle2.PyStoreArray('TRGCDC2DFinderFastTracks')

    def event(self):
        if len(self.trk2d) == 0 or len(self.fast_trk2d) == 0:
            return
        ts_hits = self.trk2d[0].getRelationsTo('CDCTriggerSegmentHits')
        B2DEBUG(10, 'TS size: {}'.format(ts_hits.size()))
        fast_ts_hits = self.fast_trk2d[0].getRelationsTo('CDCTriggerSegmentHits')
        B2DEBUG(10, 'fast TS size: {}'.format(fast_ts_hits.size()))


main.add_module(Ana())
main.add_module('RootOutput', outputFileName='cdctrigger.root')

# input('paused')
# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
