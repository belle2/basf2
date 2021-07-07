#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import sys
import math
import basf2

# reenable GUI thread for our canvas
from ROOT import PyConfig
PyConfig.StartGuiThread = True  # noqa

# Load the required libraries
import ROOT
from ROOT import Belle2

# Set the global log level
basf2.logging.log_level = basf2.LogLevel.WARNING

# Create some histograms to be filled
h_nTracks = ROOT.TH1D('nTracks', 'Number of Tracks per Event;#', 20, 0, 20)
h_pdg = ROOT.TH1D('pid', 'Particle code of particles', 100, -50, 50)
h_momentum = ROOT.TH1D('momentum', 'Momentum of particles', 200, 0, 8)
h_pt = ROOT.TH1D('pt', 'Transverse Momentum of particles', 200, 0, 2)
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


class ShowMCParticles(basf2.Module):

    """Simple module to collect some information about MCParticles"""

    def event(self):
        """Fill the histograms with the values of the MCParticle collection"""

        mcParticles = Belle2.PyStoreArray('MCParticles')
        nTracks = mcParticles.getEntries()
        h_nTracks.Fill(nTracks)
        for i in range(nTracks):
            mc = mcParticles[i]
            if mc.hasStatus(Belle2.MCParticle.c_PrimaryParticle):
                p = mc.getMomentum()
                h_momentum.Fill(p.Mag())
                h_pt.Fill(p.Perp())
                h_phi.Fill(p.Phi() / math.pi * 180)
                h_theta.Fill(p.Theta() / math.pi * 180)
                h_costheta.Fill(math.cos(p.Theta()))
                h_pdg.Fill(mc.getPDG())
                h_vertex.Fill(mc.getProductionVertex().X(),
                              mc.getProductionVertex().Y())


main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# Register the BHWideInput module
koralw = basf2.register_module('KoralWInput')

# Set the logging level for the KoralW module to INFO in order to see the total
# cross section
koralw.set_log_level(basf2.LogLevel.INFO)

# Register the Progress module and the Python histogram module
progress = basf2.register_module('Progress')
showMCPart = ShowMCParticles()

# Create the main path and add the modules
main.add_module(progress)
main.add_module(koralw)
main.add_module(showMCPart)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)

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
