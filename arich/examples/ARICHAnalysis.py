#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run myArichModule.
# It uses ParicleGun module to generate tracks,
# (see "generators/example/ParticleGunFull.py" for detailed usage),
# simulated inner detectors (SVD, PXD, CDC), and needs
# reconstructed tracks(MCTrackFinder+GenFit) extrapolated to ARICH (ExtHits).
# The module builds geometry, performs geant4 simulation, does ARICH
# reconstruction and stores output in an output root file.
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

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [nevents], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')

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
particlegun.param('momentumParams', [0.5, 5])
# Setting the parameters for the random generation
# of the particle polar angle:
particlegun.param('thetaGeneration', 'uniformCosinus')
particlegun.param('thetaParams', [17, 35])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
# Print the parameters of the particle gun
print_params(particlegun)
# ============================================================================

# Run simulation
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', 1)
# Here you can select visualization driver and visualization commands.
# You can use any visualization supported by geant4:
# "http://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/
#  ForApplicationDeveloper/html/ch08.html"
# Uncomment following lines  to create VRML file that can be examined with any
# vrml viewer (freewrl,...)
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                            '/vis/scene/add/axes 0 0 0 100 mm',
#                            '/vis/scene/add/trajectories smooth',
#                            '/vis/modeling/trajectories/create/drawByCharge'])
# =============================================================================

# CDC digitizer
cdcDigitizer = register_module('CDCDigitizer')

# MC track finder (for simplicity)
mctrackfinder = register_module('MCTrackFinder')

# Track fitting
cdcfitting = register_module('GenFitter')

# Track extrapolation
ext = register_module('Ext')

# ARICH digitization module
arichDIGI = register_module('ARICHDigitizer')

# ARICH reconstruction module
arichRECO = register_module('ARICHReconstructor')
arichRECO.logging.log_level = LogLevel.DEBUG
arichRECO.logging.debug_level = 20
arichRECO.param('inputTrackType', 0)

# my module - reconstruction efficiency analysis
arichEfficiency = register_module('ARICHAnalysis')
arichEfficiency.logging.log_level = LogLevel.DEBUG
arichEfficiency.logging.debug_level = 20
arichEfficiency.param('outputFile', filename)
arichEfficiency.param('inputTrackType', 0)

# Do the simulation
# =============================================================================
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(cdcfitting)
main.add_module(ext)
main.add_module(arichDIGI)
main.add_module(arichRECO)
main.add_module(arichEfficiency)

# Process events
process(main)

# Print call statistics
print statistics
