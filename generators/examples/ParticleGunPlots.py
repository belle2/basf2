#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import math
from basf2 import *
logging.log_level = LogLevel.WARNING

# Load the required libraries
import ROOT
ROOT.gSystem.Load('libframework')
ROOT.gSystem.Load('libgenerators')
from ROOT import Belle2

# Create some histograms to be filled
h_nTracks = ROOT.TH1D('nTracks', 'Number of Tracks per Event;#', 200, 0, 100)
h_pdg = ROOT.TH1D('pid', 'Particle code of particles', 500, -250, 250)
h_momentum = ROOT.TH1D('momentum', 'Momentum of particles', 200, 0, 10)
h_pt = ROOT.TH1D('pt', 'Transverse Momentum of particles', 200, 0, 10)
h_phi = ROOT.TH1D('phi', 'Azimuth angle of particles', 200, -180, 180)
h_theta = ROOT.TH1D('theta', 'Polar angle of particles', 200, 0, 180)
h_costheta = ROOT.TH1D('costheta', 'Cosinus of the polar angle of particles',
                       200, -1, 1)
h_vertex = ROOT.TH2D(
    'xyvertex',
    'XY Vertex of particles',
    200,
    -10,
    10,
    200,
    -10,
    10,
    )


class ShowMCParticles(Module):

    """Simple module to collect some information about MCParticles"""

    def __init__(self):
        """Initialize the module"""

        super(ShowMCParticles, self).__init__()

    def event(self):
        """Fill the histograms with the values of the MCParticle collection"""

        mcParticles = Belle2.PyStoreArray('MCParticles')
        nTracks = mcParticles.getEntries()
        h_nTracks.Fill(nTracks)
        for i in range(nTracks):
            mc = mcParticles[i]
            p = mc.getMomentum()
            h_momentum.Fill(p.Mag())
            h_pt.Fill(p.Perp())
            h_phi.Fill(p.Phi() / math.pi * 180)
            h_theta.Fill(p.Theta() / math.pi * 180)
            h_costheta.Fill(math.cos(p.Theta()))
            h_pdg.Fill(mc.getPDG())
            h_vertex.Fill(mc.getProductionVertex().X(),
                          mc.getProductionVertex().Y())


eventinfosetter = register_module('EventInfoSetter')
progress = register_module('Progress')
particlegun = register_module('ParticleGun')
showMCPart = ShowMCParticles()

# Specify number of events to generate
eventinfosetter.param({'evtNumList': [10000], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({  # Generate 5 tracks But vary the number of tracks
                     # according to Poisson distribution Generate pi+, pi-, e+
                     # and e- with a normal distributed transversal momentum
                     # with a center of 5 GeV and a width of 1 GeV a normal
                     # distributed phi angle, center of 180 degree and a width
                     # of 30 degree Generate theta angles uniform in cos theta
                     # between 17 and 150 degree normal distributed vertex
                     # generation around the origin with a sigma of 2cm in the
                     # xy plane and no deviation in z all tracks sharing the
                     # same vertex per event
    'nTracks': 5,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normalPt',
    'momentumParams': [5, 1],
    'phiGeneration': 'normal',
    'phiParams': [180, 30],
    'thetaGeneration': 'uniformCosinus',
    'thetaParams': [17, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 2],
    'yVertexParams': [0, 2],
    'zVertexParams': [0, 0],
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
c = ROOT.TCanvas('Canvas', 'Canvas', 1536, 768)
c.Divide(4, 2, 1e-5, 1e-5)

# Draw all histograms
histograms = [
    h_nTracks,
    h_pdg,
    h_momentum,
    h_pt,
    h_theta,
    h_costheta,
    h_phi,
    ]
for (i, h) in enumerate(histograms):
    c.cd(i + 1)
    h.SetMinimum(0)
    h.Draw()
c.cd(i + 2)
h_vertex.Draw('colz')
c.Update()

# Wait for enter to be pressed
sys.stdin.readline()
