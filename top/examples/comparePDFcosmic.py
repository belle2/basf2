#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ----------------------------------------------------------------------------
# Example of using TOPPDFChecker to compare analytic PDF with simulation
# Note: time jitters and backgrounds are excluded to compare details easier
#
# In this example one can easily set the direction to be incoming or outcoming
# ----------------------------------------------------------------------------

import basf2 as b2
import math

# particle parameters (given in local bar frame) - change as you like
slot = 10      # slot ID
xloc = 0       # cm, local coordinate
zloc = 0       # cm, local coordinate
theta = 90     # degrees
dphi = 0       # degrees, with respect to normal impact (= 90 deg local)
direction = 'incoming'
# direction = 'outcoming'
p = 3          # GeV/c
pdg = 13       # muon
outputFile = 'comparePDF.root'  # file to save histograms

# set phi and y local according to impact direction
phi = 90 - dphi
yloc = -1.1  # inner bar surface - 1 mm
if direction == 'incoming':
    phi += 180
    yloc = 1.1  # outer bar surface + 1 mm

# transform to Belle II frame
alpha = (slot - 0.5) * 360 / 16 - 90  # rotation angle
ca = math.cos(math.radians(alpha))
sa = math.sin(math.radians(alpha))
x = xloc * ca - (yloc + 120) * sa
y = xloc * sa + (yloc + 120) * ca
z = zloc + 60.5
phi += alpha

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
particlegun.param('xVertexParams', [x])
particlegun.param('yVertexParams', [y])
particlegun.param('zVertexParams', [z])
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
