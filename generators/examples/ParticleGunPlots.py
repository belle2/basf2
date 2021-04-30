#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import create_path, register_module, process, logging, \
    LogLevel, Module, statistics

# Load the required libraries
from ROOT import TH1D, TH2D, TCanvas, Belle2, PyConfig

logging.log_level = LogLevel.WARNING
# reenable GUI thread for our canvas
PyConfig.StartGuiThread = True

# Create some histograms to be filled
h_nTracks = TH1D('nTracks', 'Number of Tracks per Event', 100, 0, 50)
h_pdg = TH1D('pid', 'PDG codes', 500, -250, 250)
h_momentum = TH1D('momentum', 'Momentum p; p/GeV', 200, 0, 10)
h_pt = TH1D('pt', 'Transverse Momentum p_{t};p_{t}/GeV', 200, 0, 10)
h_phi = TH1D('phi', 'azimuthal angle #phi; #phi in degree', 200, -180, 180)
h_theta = TH1D('theta', 'polar angle #theta; #theta in degree', 200, 0, 180)
h_costheta = TH1D('costheta', 'cos(#theta); cos(#theta)', 200, -1, 1)
h_xyvertex = TH2D('xyvertex', 'vertex in xy;x/#mum;y/#mum',
                  500, -500, 500, 500, -500, 500)
h_zxvertex = TH2D('zxvertex', 'vertex in zx;z/#mum;x/#mum',
                  500, -500, 500, 500, -500, 500)
h_zyvertex = TH2D('zyvertex', 'vertex in zy;z/#mum;y/#mum',
                  500, -500, 500, 500, -500, 500)


class ShowMCParticles(Module):
    """Simple module to collect some information about MCParticles"""

    def event(self):
        """Fill the histograms with the values of the MCParticle collection"""
        mcParticles = Belle2.PyStoreArray('MCParticles')
        h_nTracks.Fill(mcParticles.getEntries())
        for mc in mcParticles:
            p = mc.getMomentum()
            h_momentum.Fill(p.Mag())
            h_pt.Fill(p.Perp())
            h_phi.Fill(p.Phi() / math.pi * 180)
            h_theta.Fill(p.Theta() / math.pi * 180)
            h_costheta.Fill(math.cos(p.Theta()))
            h_pdg.Fill(mc.getPDG())
            h_xyvertex.Fill(mc.getProductionVertex().X() * 1e4,
                            mc.getProductionVertex().Y() * 1e4)
            h_zxvertex.Fill(mc.getProductionVertex().Z() * 1e4,
                            mc.getProductionVertex().X() * 1e4)
            h_zyvertex.Fill(mc.getProductionVertex().Z() * 1e4,
                            mc.getProductionVertex().Y() * 1e4)


# Create Modules
eventinfosetter = register_module('EventInfoSetter')
progress = register_module('Progress')
particlegun = register_module('ParticleGun')
vertexsmear = register_module('SmearPrimaryVertex')
showMCPart = ShowMCParticles()

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [10000], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({
    # Generate 5 tracks
    'nTracks': 5,
    # But vary the number of tracks according to Poisson distribution
    'varyNTracks': True,
    # Generate pi+, pi-, e+ and e-
    'pdgCodes': [211, -211, 11, -11],
    # having a normal distributed transversal momentum
    'momentumGeneration': 'normalPt',
    # with a mean of 5 GeV and a width of 1 GeV
    'momentumParams': [5.0, 1.0],
    # use normal distribution for phi
    'phiGeneration': 'normal',
    # with a mean of 90 degree and w width of 30 degree
    'phiParams': [90, 30],
    # and generate theta uniform in cos(theta)
    'thetaGeneration': 'uniformCos',
    # between 17 and 150 degree
    'thetaParams': [17, 150],
    # finally, vertex generation, fixed, we use smearing module
    'vertexGeneration': 'fixed',
    'xVertexParams': [0],
    'yVertexParams': [0],
    'zVertexParams': [0],
    # all tracks should originate from the same vertex
    'independentVertices': False,
})


# The beamspot size is determined by using the beamsize for LER and HER from
# the TDR, multiply both zx-distributions including their crossing angle and
# fit a rotated 2D gaussian to the resulting shape.
# Parameters used (angle is the angle between beam and z axis):
# HER: sigma_x = 10.2µm, sigma_z = 6mm, angle= 41.5mrad
# LER: sigma_x = 7.76µm, sigma_z = 5mm, angle=-41.5mrad
vertexsmear.param({
    # Beamspot size in x
    'sigma_pvx': 6.18e-4,  # µm
    # Beamspot size in y
    'sigma_pvy': 59e-7,    # nm
    # Beamspot size in z
    'sigma_pvz': 154e-4,   # µm
    # Angle between beamspot and z axis, rotation around y
    'angle_pv_zx': -1.11e-2,
})


# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(vertexsmear)
main.add_module(showMCPart)

# generate events
process(main)

# show call statistics
print(statistics)

# Create a Canvas to show histograms
c = TCanvas('Canvas', 'Canvas', 1920, 768)
c.Divide(5, 2, 1e-5, 1e-5)

# Draw all histograms
histograms = [h_nTracks, h_pdg, h_momentum, h_pt, h_theta, h_costheta, h_phi]
vertexhists = [h_xyvertex, h_zxvertex, h_zyvertex]
for (i, h) in enumerate(histograms):
    c.cd(i + 1)
    h.SetMinimum(0)
    h.Draw()
for (j, h) in enumerate(vertexhists, i + 2):
    c.cd(j)
    h.Draw('colz')
c.Update()

# Wait for enter to be pressed
print("Press Enter to exit ...")
sys.stdin.readline()
