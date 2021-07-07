#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# -------------------------------------------------------------------------
# Example of using TOPPDFChecker to compare analytic PDF with simulation
# Note: time jitters and backgrounds are excluded to compare details easier
# -------------------------------------------------------------------------

import basf2 as b2

# particle parameters (emitted from IP) - change as you like
p = 3       # GeV/c
theta = 90  # degrees
phi = 20    # degrees
pdg = 211   # pion
outputFile = 'comparePDF.root'  # file to save histograms

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.WARNING)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', outputFile)  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP and B-field)
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['MagneticField', 'TOP'])
main.add_module(geometry)

# Particle gun: generate single particle w/ fixed vertex, momentum and kind
particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [pdg])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [p])
particlegun.param('thetaGeneration', 'fixed')
particlegun.param('thetaParams', [theta])
particlegun.param('phiGeneration', 'fixed')
particlegun.param('phiParams', [phi])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
particlegun.param('independentVertices', False)
main.add_module(particlegun)

# Simulation
simulation = b2.register_module('FullSim')
main.add_module(simulation)

# TOP digitization: all time jitters turned OFF
topdigi = b2.register_module('TOPDigitizer')
topdigi.param('useWaveforms', False)
topdigi.param('simulateTTS', False)
topdigi.param('electronicJitter', 0.0)
topdigi.param('timeZeroJitter', 0.0)
main.add_module(topdigi)

# Dedicated track maker using MC information only
trackmaker = b2.register_module('TOPMCTrackMaker')
main.add_module(trackmaker)

# TOP PDF: time jitters are excluded
toppdf = b2.register_module('TOPPDFChecker')
main.add_module(toppdf)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
