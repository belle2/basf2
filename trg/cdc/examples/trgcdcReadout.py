#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
import numpy as np
import os
import glob
from ROOT import gROOT, Belle2

from cdctrigger import add_cdc_trigger

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
bkgdir = '/sw/belle2/bkg/'

# ------------------------- #
# create path up to trigger #
# ------------------------- #

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

# ----------- #
# CDC trigger #
# ----------- #

add_cdc_trigger(main)


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
        print("CDC trigger readout")
        print("event time:", Belle2.PyStoreObj("CDCTriggerEventTime").obj().getTiming())
        tracks2Dfinder = Belle2.PyStoreArray("Trg2DFinderTracks")
        tracks2Dfitter = Belle2.PyStoreArray("Trg2DFitterTracks")
        tracks3Dfitter = Belle2.PyStoreArray("Trg3DFitterTracks")
        tracks3Dneuro = Belle2.PyStoreArray("TrgNNTracks")
        listnames = ["2D finder", "2D fitter", "3D fitter", "Neurotrigger"]
        for i, tracks in enumerate([tracks2Dfinder, tracks2Dfitter,
                                    tracks3Dfitter, tracks3Dneuro]):
            print(listnames[i], "has", len(tracks), "tracks.")
            for track in tracks:
                print(f"phi0[deg] = {track.getPhi0() * 180.0 / np.pi:.2f}",
                      f"pt[GeV] = {track.getTransverseMomentum(1.5):.3f}",
                      f"charge = {int(track.getChargeSign())}",
                      f"theta[deg] = {np.arctan2(1.0, track.getCotTheta()) * 180.0 / np.pi:.2f}",
                      f"z[cm] = {track.getZ0():.2f}")
        print(len(Belle2.PyStoreArray("MCParticles")), "MCParticles.")
        for particle in Belle2.PyStoreArray("MCParticles"):
            print(f"phi0[deg] = {particle.getMomentum().Phi() * 180.0 / np.pi:.2f}",
                  f"pt[GeV] = {particle.getMomentum().Pt():.3f}",
                  f"charge = {int(particle.getCharge())}",
                  f"theta[deg] = {particle.getMomentum().Theta() * 180.0 / np.pi:.2f}",
                  f"z[cm] = {particle.getProductionVertex().Z():.2f}")


main.add_module(TestModule())

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
