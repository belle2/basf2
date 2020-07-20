#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from enum import Enum
from argparse import ArgumentParser
from basf2 import *
import ROOT
from pandas import DataFrame
import pandas as pd
from ROOT import Belle2
from tracking import add_tracking_reconstruction, add_cr_tracking_reconstruction
from svd import add_svd_reconstruction, add_svd_simulation
from pxd import add_pxd_reconstruction, add_pxd_simulation
from simulation import add_simulation
from reconstruction import add_reconstruction
import numpy as np
import os
import awkward as awk
import h5py


# command line options
ap = ArgumentParser('')
ap.add_argument('--output', '-o', default='output.awkd', help='output filename')
ap.add_argument('--particle',       type=int,   default=13,   help='pdg code of the particles to generate')
ap.add_argument('--momentum_lower', type=float, default=2.,   help='lower bound for momentum')
ap.add_argument('--momentum_upper', type=float, default=2.,   help='upper bound for momentum')
ap.add_argument('--phi_lower',      type=float, default=0.,   help='lower bound for phi')
ap.add_argument('--phi_upper',      type=float, default=360., help='upper bound for phi')
ap.add_argument('--theta_lower',    type=float, default=32.6, help='lower bound for theta')
ap.add_argument('--theta_upper',    type=float, default=122., help='upper bound for theta')
ap.add_argument('--xVertex',        type=float, default=0.,   help='x-coordinate of initial position')
ap.add_argument('--yVertex',        type=float, default=0.,   help='y-coordinate of initial position')
ap.add_argument('--zVertex',        type=float, default=0.,   help='z-coordinate of initial position')

opts = ap.parse_args()

# use a python3 enum for readable particle gun configuration setup


class ParticleGunConfig(Enum):
    FROMORIGIN = 0
    FROMABOVE = 1
    FROMSIDE = 2


class WritePDFs(Module):
    def initialize(self):
        self.jobid = int(os.environ.get("SLURM_ARRAY_TASK_ID", "0"))
        self.index = self.jobid * 100000
        self.trainData = []
        self.pixelData = []
        self.xAxis = ROOT.TVector3(1, 0, 0)
        self.yAxis = ROOT.TVector3(0, 1, 0)
        self.zAxis = ROOT.TVector3(0, 0, 1)

    def event(self):
        mcps = Belle2.PyStoreArray("MCParticles")
        mcp = sorted(mcps, key=lambda x: x.getEnergy(), reverse=True)[0]
        mcp_pdg = abs(mcp.getPDG())
        track = mcp.getRelated('Tracks')
        if not track:
            print('no related track')
            return
        logl = track.getRelated('TOPLikelihoods')
        if not logl:
            print('no related loglihoods')
            return
        pixlogl = track.getRelated('TOPPixelLikelihoods')
        if not pixlogl:
            print('no related pixel loglihoods')
            return
        digits = Belle2.PyStoreArray("TOPDigits")
        if not digits:
            print('no digits')
            return

        moduleID = 0
        photon_emissions = []
        for d in digits:
            if moduleID == 0:
                moduleID = d.getModuleID()
            simhit = d.getRelated("TOPSimHits")
            if not simhit:
                continue
            photon = simhit.getRelated("TOPSimPhotons")
            if not photon:
                continue

            chanID = d.getPixelID() - 1  # pixelID is 1-based
            time = photon.getDetectionTime()
            point = photon.getEmissionPoint()
            emitTime = photon.getEmissionTime()
            photon_emissions.append([
                chanID, time,
                point.X(), point.Y(), point.Z(), emitTime
            ])

        self.pixelData.append(
            awk.fromiter([
                np.array(pixlogl.getPixelLogL_e()),
                np.array(pixlogl.getPixelLogL_mu()),
                np.array(pixlogl.getPixelLogL_pi()),
                np.array(pixlogl.getPixelLogL_K()),
                np.array(pixlogl.getPixelLogL_p()),
                np.array(pixlogl.getPixelSigPhot_e()),
                np.array(pixlogl.getPixelSigPhot_mu()),
                np.array(pixlogl.getPixelSigPhot_pi()),
                np.array(pixlogl.getPixelSigPhot_K()),
                np.array(pixlogl.getPixelSigPhot_p()),
                logl.getLogL_e(),
                logl.getLogL_mu(),
                logl.getLogL_pi(),
                logl.getLogL_K(),
                logl.getLogL_p(),
                photon_emissions,
                mcp_pdg
            ])
        )

    def terminate(self):
        awk.save(opts.output, self.pixelData, mode="w")

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
# Number of events reset to one in evtNumList: [1]
eventinfosetter.param({'evtNumList': [10000], 'runList': [int(os.environ.get("LSB_JOBINDEX", "1"))]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
main.add_module(geometry)
geometry.param('components', [
        'MagneticField',
        'BeamPipe',
        'PXD',
        'SVD',
        'CDC',
        'TOP',
])

# Particle gun: generate multiple tracks
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [opts.particle])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('independentVertices', False)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [opts.momentum_lower, opts.momentum_upper])
particlegun.param('thetaGeneration', 'uniform')  # 'uniformCos'
particlegun.param('thetaParams', [opts.theta_lower, opts.theta_upper])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [opts.phi_lower, opts.phi_upper])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [opts.xVertex])
particlegun.param('yVertexParams', [opts.yVertex])
particlegun.param('zVertexParams', [opts.zVertex])


main.add_module(particlegun)
# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

add_svd_simulation(main)
add_pxd_simulation(main)

# PXD digitization & clustering
add_pxd_reconstruction(main)

# SVD digitization & clustering
add_svd_reconstruction(main)

# CDC digitization
cdcDigitizer = register_module('CDCDigitizer')
main.add_module(cdcDigitizer)

# TOP digitization
topdigi = register_module('TOPDigitizer')
main.add_module(topdigi)

add_tracking_reconstruction(main)

# Track extrapolation
ext = register_module('Ext')
main.add_module(ext)

# TOP reconstruction
top_cm = register_module('TOPChannelMasker')
main.add_module(top_cm)
topreco = register_module('TOPReconstructor')
# topreco.logging.log_level = LogLevel.DEBUG  # remove or comment to suppress printout
# topreco.logging.debug_level = 2  # or set level to 0 to suppress printout
main.add_module(topreco)

pdfdebug = register_module("TOPPDFDebugger")
pdfdebug.pdfOption = 'fine'  # other options: 'optimal', 'rough'
main.add_module(pdfdebug)

fcp = WritePDFs()
fcp.fname = opts.output
main.add_module(fcp)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
# print(statistics)
