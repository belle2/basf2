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
<header>
  <output>TOPNtupleRecoTest.root</output>
  <contact>marko.staric@ijs.si</contact>
  <description>Makes a flat ntuple for validation of top reconstruction </description>
</header>
"""

import basf2 as b2

# ---------------------------------------------------------------------------------
# Simulate, reconstruct and make a flat ntuple for validation of top reconstruction
# Here we just want to test intrinsic TOP PID (no other material and ideal tracks)
# ---------------------------------------------------------------------------------

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

b2.set_random_seed(123452)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])
main.add_module(eventinfosetter)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP and B-field, since we are testing intrinsic TOP PID)
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['MagneticField', 'TOP'])
main.add_module(geometry)

# Particle gun: generate multiple tracks
particlegun = b2.register_module('ParticleGun')
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
simulation = b2.register_module('FullSim')
main.add_module(simulation)

# TOP digitization
topdigi = b2.register_module('TOPDigitizer')
main.add_module(topdigi)

# Dedicated track maker using MC information only
trackmaker = b2.register_module('TOPMCTrackMaker')
main.add_module(trackmaker)

# Channel masker
main.add_module('TOPChannelMasker')

# TOP reconstruction
topreco = b2.register_module('TOPReconstructor')
main.add_module(topreco)

# Output: make flat ntuple from TOPLikelihoods, tracking info and MC truth
output = b2.register_module('TOPNtuple')
output.param('outputFileName', '../TOPNtupleRecoTest.root')
main.add_module(output)

# Show progress of processing
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
