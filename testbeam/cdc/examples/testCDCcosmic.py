#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# simulation

# Number of events to be generated
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfosetter.param('runList', [1])
eventinfosetter.param('expList', [1])
main.add_module(eventinfosetter)

# Particle gun: generate single muons coming from above
# Note: current cosmic generator (module: Cosmisc) is not designed for this purpose
#       (distributions valid only for full Belle II w/ magnetic field)
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 1)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0, 10])
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [90, 0])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [-90, 0])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [200])
particlegun.param('zVertexParams', [0])
main.add_module(particlegun)

# geometry parameter database
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/CDCcosmicTests.xml')
gearbox.param('override', [("/DetectorComponent[@name='TOP']//Nbar", '1', ''),
                           ("/DetectorComponent[@name='TOP']//Phi0", '90', 'deg')])
main.add_module(gearbox)

# detector geometry
# available components:
#    'MagneticField'     small magnetic field (0, 0, 1e-4) Tesla
#    'CDC'               default geometry
#    'TOP'               single module only (on top of CDC)
#    'CDCcosmicTrigger'  two scntillators + 5mm Pb plate in between, inside central hole
geometry = register_module('Geometry')
components = []
# components = ['MagneticField', 'CDC', 'CDCcosmicTrigger'] # to speed up g4sim
geometry.param('components', components)
main.add_module(geometry)

# detector simulation
g4sim = register_module('FullSim')
main.add_module(g4sim)

# trigger simulation
trigger = register_module('TOPbeamTrigger')
trigger.param('detectorIDs', [1, 2])  # scintillator counters 1 and 2
trigger.param('thresholds', [0.5, 0.5])  # MeV
main.add_module(trigger)
emptyPath = create_path()
trigger.if_false(emptyPath)

# CDC digitization
cdc_digitizer = register_module('CDCDigitizer')
main.add_module(cdc_digitizer)

# TOP digitization
if components == [] or 'TOP' in components:
    top_digitizer = register_module('TOPDigitizer')
    main.add_module(top_digitizer)

# Reconstruction

# Material effects for all track extrapolations
material_effects = register_module('SetupGenfitExtrapolation')
material_effects.param('whichGeometry', 'TGeo')
main.add_module(material_effects)

# CDC track finder (Trasan or TrackFinderCDCCosmics)
cdc_trackfinder = register_module('Trasan')
# cdc_trackfinder = register_module('TrackFinderCDCCosmics')
main.add_module(cdc_trackfinder)

# track fitting
trackfitter = register_module('GenFitter')
trackfitter.param('PDGCodes', [13])
main.add_module(trackfitter)

# display
# display = main.add_module('Display') # or AsyncDisplay
# display.param('showCDCHits', True)
# display.param('fullGeometry', True)

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'testCDCcosmic.root')
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
