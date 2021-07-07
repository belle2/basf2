#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from optparse import OptionParser
import os
# Options from command line
parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=100,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename', default='ARICHDQM.root')
parser.add_option('-d', '--debug', dest='debugLevel', default=10)
parser.add_option('-s', '--seed', dest='seed', default=111111)
(options, args) = parser.parse_args()

nevents = int(options.nevents)
debugLevel = int(options.debugLevel)
seed = int(options.seed)
# suppress messages and warnings during processing DEBUG, INFO, WARNING, ERROR
b2.set_log_level(b2.LogLevel.INFO)

home = os.environ['BELLE2_LOCAL_DIR']
# cosmic test local DB folder
b2.use_local_database(home + "/arich/database/cosmicTest_payloads/cosmicTest_database.txt",
                      home + "/arich/database/cosmicTest_payloads")

# Create path
main = b2.create_path()

# Create Event information
main.add_module('EventInfoSetter', evtNumList=nevents, logLevel=b2.LogLevel.DEBUG)

# Histogram manager module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', options.filename)  # File to save histograms
main.add_module(histo)


# Load parameters
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Create geometry
geometry = b2.register_module('Geometry')
geometry.param('components', ['ARICH'])
# build from DB
geometry.param('useDB', 1)
main.add_module(geometry)

# Particle gun module
particlegun = b2.register_module('ParticleGun')
# Setting the random seed for particle generation:
b2.set_random_seed(seed)
# Setting the list of particle codes (PDG codes) for the generated particles
particlegun.param('pdgCodes', [11])
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
b2.print_params(particlegun)
main.add_module(particlegun)

# ============================================================================

# Run simulation
simulation = b2.register_module('FullSim')
simulation.param('StoreOpticalPhotons', True)
# by default only 35% of photons are propagated, which is below QE of some HAPDs! change fraction to 45% here.
# -> change the default fraction for the release!
simulation.param('PhotonFraction', 0.45)
main.add_module(simulation)

# ARICH digitization module
arichDIGI = b2.register_module('ARICHDigitizer')
arichDIGI.param('BackgroundHits', 0)
main.add_module(arichDIGI)

# fill ARICHHits from ARICHDigits
arichHits = b2.register_module('ARICHFillHits')
main.add_module(arichHits)

# add ARICH DQM module
arichDQM = b2.register_module('ARICHDQM')
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
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)

# plot DQM histograms
com = 'root -l ' + options.filename + ' ' + home + '/arich/utility/scripts/plotDQM.C'
os.system(com)
