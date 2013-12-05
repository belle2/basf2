#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run ARICHAnalysisModule.
# It uses ParicleGun module to generate tracks,
# (see "generators/example/ParticleGunFull.py" for detailed usage),
# simulated inner detectors (PXD, SVD, CDC), and needs
# reconstructed tracks(MCTrackFinder+GenFit) extrapolated to ARICH (ExtHits).
# The module builds geometry, performs geant4 simulation, does ARICH
# reconstruction and stores output in an output root file (defined in
# ARICHAnalysisModule).
#
##############################################################################

from basf2 import *
from optparse import OptionParser

# Options from command line
parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=10,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename', default='extArichTest.root')
(options, args) = parser.parse_args()
nevents = int(options.nevents)
filename = options.filename

# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
set_log_level(LogLevel.INFO)

# Create path
main = create_path()

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevents], 'runList': [1]})
main.add_module(eventinfosetter)

# Load parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Create geometry
geometry = register_module('Geometry')
geometry.param('Components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'ARICH',
    ])
main.add_module(geometry)

# Particle gun module
particlegun = register_module('ParticleGun')
# Setting the random seed for particle generation:
set_random_seed(1028307)
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-211, 211, -321, 321])
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)
# if you set nTracks to 0, then for each PDG code in pdgCodes list a track
# will be generated on each event.
# Setting the parameters for the random generation
# of particles momenta:
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.5, 4])
# Setting the parameters for the random generation
# of the particle polar angle:
particlegun.param('thetaGeneration', 'uniformCosinus')
particlegun.param('thetaParams', [17, 35])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
particlegun.param('independentVertices', False)
# Print the parameters of the particle gun
print_params(particlegun)
main.add_module(particlegun)
# ============================================================================

# Run simulation
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', 1)
main.add_module(simulation)

# PXD digitization & clustering
pxd_digitizer = register_module('PXDDigitizer')
main.add_module(pxd_digitizer)
pxd_clusterizer = register_module('PXDClusterizer')
main.add_module(pxd_clusterizer)

# SVD digitization & clustering
svd_digitizer = register_module('SVDDigitizer')
main.add_module(svd_digitizer)
svd_clusterizer = register_module('SVDClusterizer')
main.add_module(svd_clusterizer)

# CDC digitization
cdcDigitizer = register_module('CDCDigitizer')
main.add_module(cdcDigitizer)

# MC track finder (for simplicity)
mctrackfinder = register_module('MCTrackFinder')
main.add_module(mctrackfinder)

# Track fitting
trackfitter = register_module('GenFitter')
main.add_module(trackfitter)

# Track extrapolation
ext = register_module('Ext')
main.add_module(ext)

# ARICH digitization module
arichDIGI = register_module('ARICHDigitizer')
main.add_module(arichDIGI)

# ARICH reconstruction module
arichRECO = register_module('ARICHReconstructor')
arichRECO.logging.log_level = LogLevel.DEBUG
arichRECO.logging.debug_level = 20
arichRECO.param('inputTrackType', 0)
main.add_module(arichRECO)

# my module - reconstruction efficiency analysis
arichEfficiency = register_module('ARICHAnalysis')
arichEfficiency.logging.log_level = LogLevel.DEBUG
arichEfficiency.logging.debug_level = 20
arichEfficiency.param('outputFile', filename)
arichEfficiency.param('inputTrackType', 0)
main.add_module(arichEfficiency)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics
