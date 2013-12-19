#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import math
from basf2 import create_path, register_module, process, logging, \
    LogLevel, Module, statistics
logging.log_level = LogLevel.WARNING

# Load the required libraries
from ROOT import TH1D, TH2D, TCanvas, Belle2

# Create some histograms to be filled
h_nTracks = TH1D('nTracks', 'Number of Tracks per Event', 100, 0, 50)
h_pdg = TH1D('pid', 'PDG codes', 500, -250, 250)
h_momentum = TH1D('momentum', 'Momentum p', 200, 0, 10)
h_pt = TH1D('pt', 'Transverse Momentum p_{t}', 200, 0, 10)
h_phi = TH1D('phi', '#phi', 200, -180, 180)
h_theta = TH1D('theta', '#theta', 200, 0, 180)
h_costheta = TH1D('costheta', 'cos(#theta)', 200, -1, 1)
h_vertex = TH2D('xyvertex', 'vertex in xy', 200, -10, 10, 200, -10, 10)


class ShowMCParticles(Module):
    """Simple module to collect some information about MCParticles"""
    def __init__(self):
        """Initialize the module"""
        super(ShowMCParticles, self).__init__()

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
            h_vertex.Fill(mc.getProductionVertex().X(),
                          mc.getProductionVertex().Y())

# Create Modules
eventinfosetter = register_module('EventInfoSetter')
progress = register_module('Progress')
particlegun = register_module('ParticleGun')
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
    # finally, vertex generation, normal distributed
    'vertexGeneration': 'normal',
    # with a mean of 0 and w width o 2 cm for x
    'xVertexParams': [0, 2],
    # also a mean of 0 and w width o 2 cm for y
    'yVertexParams': [0, 2],
    # and a width of zero for z
    'zVertexParams': [0, 0],
    # all tracks should originate from the same vertex
    'independentVertices': False,
})

# create processing path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(showMCPart)

# generate events
process(main)

# show call statistics
print statistics

# Create a Canvas to show histograms
c = TCanvas('Canvas', 'Canvas', 1536, 768)
c.Divide(4, 2, 1e-5, 1e-5)

# Draw all histograms
histograms = [h_nTracks, h_pdg, h_momentum, h_pt, h_theta, h_costheta, h_phi]
for (i, h) in enumerate(histograms):
    c.cd(i + 1)
    h.SetMinimum(0)
    h.Draw()
c.cd(i + 2)
h_vertex.Draw('colz')
c.Update()

# Wait for enter to be pressed
print "Press Enter to exit ..."
sys.stdin.readline()
