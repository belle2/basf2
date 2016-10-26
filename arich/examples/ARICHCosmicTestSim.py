#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from optparse import OptionParser

# Options from command line
parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=100,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename', default='ARICHHists.root')
parser.add_option('-d', '--debug', dest='debugLevel', default=10)
parser.add_option('-s', '--seed', dest='seed', default=111111)
(options, args) = parser.parse_args()

nevents = int(options.nevents)
debugLevel = int(options.debugLevel)
seed = int(options.seed)
# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
set_log_level(LogLevel.INFO)

home = os.environ['BELLE2_LOCAL_DIR']
# cosmic test local DB folder
# use_local_database(home + "/arich/database/cosmicTest_payloads/cosmicTest_database.txt",
#                   home + "/arich/database/cosmicTest_payloads")

# Create path
main = create_path()

# Create Event information
main.add_module('EventInfoSetter', evtNumList=nevents, logLevel=LogLevel.DEBUG)

# Load parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Create geometry
geometry = register_module('Geometry')
geometry.param('components', ['ARICH'])
# build from DB
geometry.param('useDB', 1)
main.add_module(geometry)

# Particle gun module
particlegun = register_module('ParticleGun')
# Setting the random seed for particle generation:
set_random_seed(seed)
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [13])
# Setting the number of tracks to be generated per event:
particlegun.param('nTracks', 1)
# if you set nTracks to 0, then for each PDG code in pdgCodes list a track
# will be generated on each event.
# Setting the parameters for the random generation
# of particles momenta:
particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [5.0])
# Setting the parameters of particle direction
particlegun.param('thetaGeneration', 'fixed')
particlegun.param('thetaParams', [95.])
particlegun.param('phiGeneration', 'fixed')
particlegun.param('phiParams', [270])

# vertex position
particlegun.param('vertexGeneration', 'fixed')
# particlegun.param('vertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-43.88])
particlegun.param('yVertexParams', [10.0])
particlegun.param('zVertexParams', [-40.0])
# Print the parameters of the particle gun
print_params(particlegun)
main.add_module(particlegun)

# ============================================================================

# Run simulation
simulation = register_module('FullSim')
simulation.param('StoreOpticalPhotons', True)
simulation.param('PhotonFraction', 0.40)
main.add_module(simulation)

# ARICH digitization module
arichDIGI = register_module('ARICHDigitizer')
main.add_module(arichDIGI)

# fill ARICHHits from ARICHDigits
arichHits = register_module('ARICHFillHits')
main.add_module(arichHits)

# fill occupancy histograms
arichDQM = register_module('ARICHDQM')
arichDQM.param('outputFileName', options.filename)
main.add_module(arichDQM)

# add display module
# display = register_module('Display')
# change to True to show the full TGeo geometry instead of simplified extract
# display.param('fullGeometry', True)
# show ARICH hits
# display.param('showARICHHits', True)
# main.add_module(display)

# store datastore objets into root file

# output = register_module('RootOutput')
# output.param('outputFileName', "rootOutput.root")
# output.param('branchNames', ['ARICHAeroHits', 'ARICHSimHits', 'ARICHDigits', 'ARICHHits'])
# main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)

com = 'root -l ' + options.filename + ' ' + home + '/arich/utility/scripts/plotOccup.C'
os.system(com)
