#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to run and test ARICH performance.
# It uses ParicleGun (or EvtGen) module to generate tracks,
# (see "generators/example/ParticleGunFull.py" for detailed usage),
# and performs simulation with [SVD, PXD, CDC, ARICH] subdetectors.
# Output root file contains all information nedded for ARICH likelihood
# reconstruction, which can be later performed using "FullDet_rec.py" script.

##############################################################################

from basf2 import *
from optparse import OptionParser

# Options from command line
parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=10,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='extArichTest_sim.root')
parser.add_option('-s', '--seed', dest='seed', default=2014)

(options, args) = parser.parse_args()
nevents = int(options.nevents)
filename = options.filename
seed = int(options.seed)

# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
set_log_level(LogLevel.INFO)

# Create Event information
evtmetagen = register_module('EventInfoSetter')
evtmetagen.param({'evtNumList': [nevents], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters
gearbox = register_module('Gearbox')

# Create geometry
geometry = register_module('Geometry')
geometry.param('components', [
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
set_random_seed(seed)
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [-321, 321, -211, 211])
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)
# if you set nTracks to 0, then for each PDG code in pdgCodes list a track
# will be generated on each event.
# Setting the parameters for the random generation
# of particles momenta:
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.5, 3.5])
# Setting the parameters for the random generation
# of the particle polar angle:
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [17, 35])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
# Print the parameters of the particle gun
print_params(particlegun)
# ============================================================================

# setting input file for evtgen module
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', 'arich/examples/B2kpi.dec')
# ============================

# Run simulation
simulation = register_module('FullSim')
simulation.param('StoreAllSecondaries', 1)
# don't store secondaries with E<0.5 GeV
simulation.param('SecondariesEnergyCut', 500)

# CDC digitizer
cdcDigitizer = register_module('CDCDigitizer')

# MC track finder (for simplicity)
mctrackfinder = register_module('TrackFinderMCTruth')
# perform tracking only for particles that hit ARICH, and with E>0.5 GeV
mctrackfinder.param('WhichParticles', ['primary', 'ARICH'])
mctrackfinder.param('EnergyCut', 0.5)

# Track fitting
cdcfitting = register_module('GenFitter')

# Track extrapolation
ext = register_module('Ext')

#
arichRELATE = register_module('ARICHRelate')

# store output information
output = register_module('RootOutput')
output.param('outputFileName', filename)
output.param('branchNames', ['ARICHAeroHits', 'ARICHSimHits', 'ExtHits',
             'ARICHAeroHitsToExtHits'])

# Do the simulation
# =============================================================================
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
# select particle gun or evtgen
main.add_module(particlegun)
# main.add_module(evtgeninput)

main.add_module(simulation)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(cdcfitting)
main.add_module(ext)
main.add_module(arichRELATE)
main.add_module(output)

# Process events
process(main)

# Print call statistics
print statistics
