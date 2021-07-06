#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
genPixelLikelihoods.py

A sample script that demonstrates how to collect data using the TOPPixelLikelihood
data object.

Example usage:
    basf2 genPixelLikelihoods.py -n100 -- --particle 321 --output kaon.awkd

Note: data writing uses AwkwardArrays (tested with v0.12.6).
"""

import basf2 as b2
from ROOT import Belle2
from tracking import add_tracking_reconstruction
from svd import add_svd_reconstruction, add_svd_simulation
from pxd import add_pxd_reconstruction, add_pxd_simulation

from argparse import ArgumentParser
import numpy as np
import awkward as awk


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


class WriteData(b2.Module):
    """
    Data collector module. Gathers TOP Cherenkov photon data for each event.
    On terminate(), writes gathered data to given output file.

    Attributes:
        data: A list containing AwkwardArrays with each event's data.
    """

    def initialize(self):
        """Inits WriteData object."""
        #: A list containing arrays with each event's data
        self.data = []

    def event(self):
        """Collects event data about TOP Cherenkov photons."""
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
        photon_detections = []
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
            photon_detections.append([
                chanID, time,
                point.X(), point.Y(), point.Z(), emitTime
            ])

        self.data.append(
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
                photon_detections,
                mcp_pdg
            ])
        )

    def terminate(self):
        """Writes data to given output file."""
        awk.save(opts.output, self.data, mode="w")


# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10000]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
main.add_module(geometry)

# Particle gun: generate multiple tracks
particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [opts.particle])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('independentVertices', False)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [opts.momentum_lower, opts.momentum_upper])
particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [opts.theta_lower, opts.theta_upper])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [opts.phi_lower, opts.phi_upper])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [opts.xVertex])
particlegun.param('yVertexParams', [opts.yVertex])
particlegun.param('zVertexParams', [opts.zVertex])
main.add_module(particlegun)

# Simulation
simulation = b2.register_module('FullSim')
main.add_module(simulation)

add_svd_simulation(main)
add_pxd_simulation(main)

# PXD digitization & clustering
add_pxd_reconstruction(main)

# SVD digitization & clustering
add_svd_reconstruction(main)

# CDC digitization
cdcDigitizer = b2.register_module('CDCDigitizer')
main.add_module(cdcDigitizer)

# TOP digitization
topdigi = b2.register_module('TOPDigitizer')
main.add_module(topdigi)

add_tracking_reconstruction(main)

# Track extrapolation
ext = b2.register_module('Ext')
main.add_module(ext)

# TOP reconstruction
top_cm = b2.register_module('TOPChannelMasker')
main.add_module(top_cm)

topreco = b2.register_module('TOPReconstructor')
main.add_module(topreco)

pdfdebug = b2.register_module("TOPPDFDebugger")
pdfdebug.pdfOption = 'fine'  # other options: 'optimal', 'rough'
main.add_module(pdfdebug)

data_getter = WriteData()
main.add_module(data_getter)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)
