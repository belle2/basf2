#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import numpy as np
import os
import sys
import glob
from ROOT import gROOT, Belle2
gROOT.ProcessLine("gErrorIgnoreLevel = 4000;")  # ignore endless root errors for background files...

"""
Example script for the CDC trigger.
The path contains a particle gun with single tracks,
simulation up to the CDC, CDC digitizer and trigger.
Last is a short python test module to demonstrate the readout of the trigger tracks.
"""

# ------------ #
# user options #
# ------------ #

# general options
seed = 10000
evtnum = 100
usebkg = False
simplext = False  # switch for CDCDigitizer simple / realistic x-t
particlegun_params = {
    'pdgCodes': [-13, 13],
    'nTracks': 1,
    'momentumGeneration': 'inversePt',
    'momentumParams': [0.3, 10.],
    'thetaGeneration': 'uniform',
    'thetaParams': [35, 123],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0, 0.0],
    'yVertexParams': [0, 0.0],
    'zVertexParams': [-50.0, 50.0]}
basf2datadir = os.path.join(os.environ.get('BELLE2_LOCAL_DIR', None), 'data')
bkgdir = '/sw/belle2/bkg/'

# ------------------------------ #
# create path up to neurotrigger #
# ------------------------------ #

# set random seed
basf2.set_random_seed(seed)
# suppress messages and warnings during processing:
basf2.set_log_level(basf2.LogLevel.ERROR)

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
if usebkg:
    bkgmixer = basf2.register_module('BeamBkgMixer')
    bkgfiles = glob.glob(os.path.join(bkgdir, '*[!(PXD)(ECL)]??.root'))
    bkgmixer.param('backgroundFiles', bkgfiles)
    bkgmixer.param('components', ['CDC'])
    main.add_module(bkgmixer)
cdcdigitizer = basf2.register_module('CDCDigitizer')
cdcdigitizer_params = {
    'UseSimpleDigitization': simplext,
    'DoSmearing': not simplext}
cdcdigitizer.param(cdcdigitizer_params)
main.add_module(cdcdigitizer)
trgcdc = basf2.register_module('TRGCDC')
trgcdc_params = {
    'ConfigFile': os.path.join(basf2datadir, "trg/cdc/TRGCDCConfig_0_20101111.dat"),
    'InnerTSLUTFile': os.path.join(basf2datadir, "trg/cdc/innerLUT_v2.2.coe"),
    'OuterTSLUTFile': os.path.join(basf2datadir, "trg/cdc/outerLUT_v2.2.coe"),
    'HoughFinderMappingFileMinus': os.path.join(basf2datadir, "trg/cdc/HoughMappingMinus20160217.dat"),
    'HoughFinderMappingFilePlus': os.path.join(basf2datadir, "trg/cdc/HoughMappingPlus20160217.dat")}
trgcdc.param(trgcdc_params)
main.add_module(trgcdc)
neuro = basf2.register_module('NeuroTrigger')
neuro.param('filename', os.path.join(basf2datadir, "trg/cdc/Neuro20160118Nonlin.root"))
main.add_module(neuro)


# ----------- #
# test module #
# ----------- #

class TestModule(basf2.Module):
    """
    short test module to demonstrate the readout of the CDC trigger
    """

    def event(self):
        """
        print output values of the different stages in the CDC trigger
        """
        tracks = Belle2.PyStoreArray("CDCTriggerTracks")
        print("CDC trigger found", len(tracks), "tracks.")
        for track in tracks:
            print("2D finder estimate: phi0 =", track.getHoughPhiVertex() * 180. / np.pi,
                  "pt =", track.getHoughPt(),
                  "charge =", track.getCharge())
            print("2D fitter estimate: phi0 =", track.getFitPhiVertex() * 180. / np.pi,
                  "pt =", track.getFitPt())
            print("3D fitter estimate: theta =", track.getFitTheta() * 180. / np.pi,
                  "z =", track.getFitZ())
            print("Neural net estimate: theta =", track.getNNTheta() * 180. / np.pi,
                  "z =", track.getNNZ())
            particle = track.getRelatedTo("MCParticles")
            print("related MCParticle: phi0 =", particle.getMomentum().Phi() * 180. / np.pi,
                  "pt =", particle.getMomentum().Pt(),
                  "charge =", particle.getCharge(),
                  "theta =", particle.getMomentum().Theta() * 180. / np.pi,
                  "z =", particle.getProductionVertex().Z())

main.add_module(TestModule())

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
