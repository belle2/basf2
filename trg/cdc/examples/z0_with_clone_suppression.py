#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from basf2 import *
from ROOT import Belle2
from cdctrigger import add_cdc_trigger
from math import pi, tan
from interactive import embed

"""
generate tracks with particle gun, simulate CDC and CDC trigger, save the output.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 10000
evtnum = 600
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

empty_path = basf2.create_path()

main.add_module('EventInfoSetter', evtNumList=evtnum)
main.add_module('Progress')
main.add_module('Gearbox')
main.add_module('Geometry', components=['BeamPipe',
                                        'PXD', 'SVD', 'CDC',
                                        'MagneticFieldConstant4LimitedRCDC'])
particlegun = basf2.register_module('ParticleGun')
particlegun.param(particlegun_params)
main.add_module(particlegun)

# z position of the two ends of the first layer used by trigger
z_SL0 = [-31 - 1.5 / tan(30 / 180. * pi), 57 + 1.5 / tan(17 / 180. * pi)]
# radius of the first layer used by trigger
r_SL0 = 18.3


class Skim(Module):
    "Reject tracks with bad combination of z0 and theta"

    def initialize(self):
        self.mc = Belle2.PyStoreArray('MCParticles')

    def event(self):
        self.return_value(0)
        z0 = self.mc[0].getVertex().Z()
        vec = self.mc[0].getMomentum()
        # skip the event if the track didn't reach SL0
        if z_SL0[0] < z0 + r_SL0 / vec.Pt() * vec.Z() < z_SL0[1]:
            self.return_value(1)


skim = Skim()
main.add_module(skim)
skim.if_false(empty_path)

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
        self.finder_trk2d = Belle2.PyStoreArray('TRGCDC2DFinderTracks')
        self.fast_finder_trk2d = Belle2.PyStoreArray('TRGCDC2DFinderFastTracks')
        self.trk2d = Belle2.PyStoreArray('TRGCDC2DFitterTracks')
        self.fast_trk2d = Belle2.PyStoreArray('TRGCDC2DFitterFastTracks')
        self.trk3d = Belle2.PyStoreArray('TRGCDC3DFitterTracks')
        self.fast_trk3d = Belle2.PyStoreArray('TRGCDC3DFitterFastTracks')
        self.trknt = Belle2.PyStoreArray('TRGCDCNeuroTracks')
        self.fast_trknt = Belle2.PyStoreArray('TRGCDCNeuroFastTracks')

        self.n2d_finder = 0
        self.n2d_fast_finder = 0
        self.n2d_fitter = 0
        self.n2d_fast_fitter = 0
        self.n3d_fitter = 0
        self.n3d_fast_fitter = 0
        self.n3d_neuro = 0
        self.n3d_fast_neuro = 0

    def event(self):
        self.n2d_finder += any(self.finder_trk2d)
        self.n2d_fitter += any(self.trk2d)
        self.n3d_fitter += any(self.trk3d)
        self.n3d_neuro += any(self.trknt)
        self.n2d_fast_finder += any(self.fast_finder_trk2d)
        self.n2d_fast_fitter += any(self.fast_trk2d)
        self.n3d_fast_fitter += any(self.fast_trk3d)
        self.n3d_fast_neuro += any(self.fast_trknt)

        if len(self.trk2d) == 0 or len(self.fast_trk2d) == 0:
            return
        ts_hits = self.trk2d[0].getRelationsTo('CDCTriggerSegmentHits')
        B2DEBUG(10, 'TS size: {}'.format(ts_hits.size()))
        fast_ts_hits = self.fast_trk2d[0].getRelationsTo('CDCTriggerSegmentHits')
        B2DEBUG(10, 'fast TS size: {}'.format(fast_ts_hits.size()))

    def terminate(self):
        total = self.n2d_finder
        fast_total = self.n2d_fast_finder
        all_numbers = ','.join(['{}'] * 8)

        B2INFO(all_numbers.format(self.n2d_finder, self.n2d_fitter, self.n3d_fitter, self.n3d_neuro,
                                  self.n2d_fast_finder, self.n2d_fast_fitter, self.n3d_fast_fitter, self.n3d_fast_neuro))

        B2INFO('2D fitter retention rate: {:.2%} (original)/ {:.2%} (fast)'.format(
            self.n2d_fitter / total, self.n2d_fast_fitter / fast_total))
        B2INFO('3D fitter retention rate: {:.2%} (original)/ {:.2%} (fast)'.format(
            self.n3d_fitter / self.n2d_fitter, self.n3d_fast_fitter / self.n2d_fast_fitter))
        B2INFO('3D neuro retention rate: {:.2%} (original)/ {:.2%} (fast)'.format(
            self.n3d_neuro / total, self.n3d_fast_neuro / fast_total))


main.add_module(Ana())
main.add_module('RootOutput', outputFileName='cdctrigger.root')

# input('paused')
# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
