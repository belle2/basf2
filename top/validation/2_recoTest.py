#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>TOPNtupleRecoTest.root</output>
  <contact>staric</contact>
  <description>Makes a flat ntuple for validation of top reconstruction </description>
</header>
"""

from basf2 import *

# ---------------------------------------------------------------------------------
# Simulate, reconstruct and make a flat ntuple for validation of top reconstruction
# ---------------------------------------------------------------------------------

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

set_random_seed(123452)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10000], 'runList': [1]})
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP and B-field)
geometry = register_module('Geometry')
geometry.param('components', ['MagneticField', 'TOP'])
main.add_module(geometry)

# Particle gun: generate multiple tracks
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211, 321, -321])
particlegun.param('nTracks', 1)
particlegun.param('varyNTracks', False)
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [3])
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [30, 122])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
main.add_module(particlegun)

# Simulation
simulation = register_module('FullSim')
main.add_module(simulation)

# TOP digitization
topdigi = register_module('TOPDigitizer')
main.add_module(topdigi)

# Dedicated track maker using MC information only
trackmaker = register_module('TOPMCTrackMaker')
main.add_module(trackmaker)

# Channel masker
main.add_module('TOPChannelMasker')

# TOP reconstruction
topreco = register_module('TOPReconstructor')
main.add_module(topreco)

# Output: make flat ntuple from TOPLikelihoods, tracking info and MC truth
output = register_module('TOPNtuple')
output.param('outputFileName', '../TOPNtupleRecoTest.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
