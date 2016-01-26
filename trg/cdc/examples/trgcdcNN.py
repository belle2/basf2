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
Example script for testing the neurotrigger.
Single tracks are generated with a particle gun, processed by the CDC digitizer
and CDC trigger, and evaluated by an ensemble of trained neural networks.
The results are collected and evaluated by a short test module.
"""

# ------------ #
# user options #
# ------------ #

# general options - bkg, detector components and digitizing should match training
seed = 10000
evtnum = 10000
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
# test options (z resolution is tested in different pt bins)
ptedges = [0.3, 0.4, 0.5, 0.7, 1., 2., 10.]
zcut = 6

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
    'DoSmearing': not simplext,
    'Threshold': -10.}
cdcdigitizer.param(cdcdigitizer_params)
main.add_module(cdcdigitizer)
trgcdc = basf2.register_module('TRGCDC')
trgcdc_params = {
    'ConfigFile': os.path.join(basf2datadir, "trg/cdc/TRGCDCConfig_0_20101111_1051.dat"),
    'InnerTSLUTFile': os.path.join(basf2datadir, "trg/cdc/innerLUT_v2.2.coe"),
    'OuterTSLUTFile': os.path.join(basf2datadir, "trg/cdc/outerLUT_v2.2.coe"),
    'SimulationMode': 1,      # only fast simulation
    'FastSimulationMode': 0,  # full Simulation, no Clock Simulation
    'TSFLogicLUT': True,
    'HoughFinderMeshX': 160,
    'HoughFinderMeshY': 26,
    'HoughFinderMappingFileMinus': os.path.join(basf2datadir, "trg/cdc/HoughMappingMinus20140808.dat"),
    'HoughFinderMappingFilePlus': os.path.join(basf2datadir, "trg/cdc/HoughMappingPlus20140807.dat")}
trgcdc.param(trgcdc_params)
main.add_module(trgcdc)

# ------------ #
# NeuroTrigger #
# ------------ #

neuro = basf2.register_module('NeuroTrigger')

# define parameters
neuro.param('filename', os.path.join(basf2datadir, "trg/cdc/Neuro20160118Nonlin.root"))
# output warnings, info and some debug output for neurotrigger module
neuro.logging.log_level = basf2.LogLevel.DEBUG
neuro.logging.debug_level = 80
basf2.logging.set_info(basf2.LogLevel.DEBUG, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)

main.add_module(neuro)

# ----------- #
# test module #
# ----------- #


class TestModule(basf2.Module):
    """
    short test module to demonstrate the readout of the neurotrigger
    """

    def initialize(self):
        """
        initialize lists to store results
        """
        #: list of output values of the neurotrigger
        self.zNN = []
        #: list of MC z values (targets for the neurotrigger)
        self.zMC = []
        #: list of MC pt values (of pt dependent evaluation)
        self.ptMC = []

    def event(self):
        """
        get neurotrigger results and corresponding MC values
        """
        tracksmc = Belle2.PyStoreArray("MCParticles")
        tracks2d = Belle2.PyStoreArray("CDCTriggerTracks")
        for itrack, track in enumerate(tracks2d):
            mcrel = track.getRelationsTo("MCParticles")
            if len(mcrel) == 0:
                continue
            wmax = 0
            for irel in range(len(mcrel)):
                w = mcrel.weight(irel)
                if w > wmax:
                    wmax = w
                    mctrack = mcrel[irel]
            if wmax < 0.5:
                continue
            if track.hasNNZ():
                self.zNN.append(track.getNNZ())
                self.zMC.append(mctrack.getProductionVertex().Z())
                self.ptMC.append(mctrack.getMomentum().Pt())

    def terminate(self):
        """
        evaluate results (very superficial, more for demonstration than real validation)
        prints the standard deviation and efficiency for a certain cut on z
        """
        self.zNN = np.array(self.zNN)
        self.zMC = np.array(self.zMC)
        self.ptMC = np.array(self.ptMC)
        for i in range(len(ptedges) - 1):
            inbin = np.where(np.logical_and(self.ptMC >= ptedges[i],
                                            self.ptMC <= ptedges[i + 1]))[0]
            print("pt in", ptedges[i:(i + 2)], ":", len(inbin))
            if len(inbin) == 0:
                continue
            # RMS
            zNN = self.zNN[inbin]
            zMC = self.zMC[inbin]
            print("sigma:", np.std(zNN - zMC))
            # efficiency
            ip = np.where(abs(zMC) <= 1)[0]
            if len(ip) > 0:
                print("zMC in +-1cm:", len(ip))
                print("efficiency (zNN in +-%.1fcm):" % zcut, 100. * len(np.where(abs(zNN[ip]) <= zcut)[0]) / len(ip))

main.add_module(TestModule())

# Process events
basf2.process(main)

# Print call statistics
print(basf2.statistics)
