#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# --------------------------------------------------------------------
# Example of using TOP reconstruction
# needs reconstructed tracks (GFTrack), extrapolated to TOP (Ext)
# log likelihoods in TOPLikelihoods
# relation from GFTracks to TOPLikelihoods
# --------------------------------------------------------------------

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Event information, set the number of events to generate
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param({'EvtNumList': [10], 'RunList': [1]})

# particle gun: multiple tracks
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211, 321, -321])
particlegun.param('nTracks', 5)
particlegun.param('varyNTracks', True)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.5, 4])
particlegun.param('thetaGeneration', 'uniformCosinus')
particlegun.param('thetaParams', [32, 122])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
particlegun.param('independentVertices', False)
print_params(particlegun)

# Show progress of processing
progress = register_module('Progress')

# Gearbox
gearbox = register_module('Gearbox')

# Geometry
geometry = register_module('Geometry')
geometry.param('Components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    ])

# Simulation
simulation = register_module('FullSim')

# CDC digitizer
cdcDigitizer = register_module('CDCDigitizer')
# use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# MC track finder (for simplicity)
mctrackfinder = register_module('MCTrackFinder')
param_mctrackfinder = {  # select which particles to use: primary particles
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'WhichParticles': ['primary'],
    }
mctrackfinder.param(param_mctrackfinder)

# Track fitting
cdcfitting = register_module('GenFitter')

# Track extrapolation
ext = register_module('Ext')
ext.param('ExtTrackCandsColName', 'ExtTrackCands')  # output from ext (DEPRECATED)
ext.param('ExtRecoHitsColName', 'ExtRecoHits')  # output from ext (DEPRECATED)

# TOP reconstruction
topdigi = register_module('TOPDigitizer')
topreco = register_module('TOPReconstructor')
topreco.param('DebugLevel', 1)  # remove this line or set to 0 to suppress printout

# Output
output = register_module('RootOutput')
output.param('outputFileName', 'TOPOutput.root')

# Create path
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(cdcfitting)
main.add_module(ext)
main.add_module(topdigi)
main.add_module(topreco)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
