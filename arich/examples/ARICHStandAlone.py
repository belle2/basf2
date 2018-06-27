#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from optparse import OptionParser

# --------------------------------------------------------------------
# Example script for ARICH stand alone simulation and reconstruction
# (only ARICH is simulated, very fast)
# Instead of tracks from CDC, the track information is taken from MC hits
# on aerogel plane (ARICHAeroHit).
# Smearing of track position and direction is applied to mimic tracking
# resolution.
#
# Author: Luka Santelj (Oct. 2016)
# --------------------------------------------------------------------

parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=1000,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='ARICHEvents.root')
(options, args) = parser.parse_args()

home = os.environ['BELLE2_LOCAL_DIR']

# use_local_database("centraldb/dbcache.txt")

# Suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [int(options.nevents)], 'runList': [1]})
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQMhistograms.root')  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
# only ARICH and magnetic field
geometry = register_module('Geometry')
geometry.param('components', [
    'MagneticField',
    'ARICH'])
main.add_module(geometry)

# Particle gun
particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211, 321, -321])
particlegun.param('nTracks', 1)
# particlegun.param('varyNTracks', True)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.5, 4])
particlegun.param('thetaGeneration', 'uniformCos')
particlegun.param('thetaParams', [17, 35])
particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [0, 360])
particlegun.param('vertexGeneration', 'fixed')
particlegun.param('xVertexParams', [0])
particlegun.param('yVertexParams', [0])
particlegun.param('zVertexParams', [0])
particlegun.param('independentVertices', False)
main.add_module(particlegun)

# Simulation
simulation = register_module('FullSim')
# Uncomment to build visualisation with HepRep
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', [
#    '/vis/open HepRepFile',
#    '/vis/scene/create',
#    '/vis/scene/add/volume',
#    '/vis/sceneHandler/attach',
#    '/vis/viewer/flush',
#    '/vis/scene/add/trajectories smooth',
#    '/vis/scene/add/hits'
# ])
main.add_module(simulation)

# Check for volume intersection/overlaps
# overlapchecker = register_module('OverlapChecker')
# main.add_module(overlapchecker)

# ARICH digitization
arichDigi = register_module('ARICHDigitizer')
main.add_module(arichDigi)

# convert ARICHDigits to ARICHHits
arichHits = register_module('ARICHFillHits')
main.add_module(arichHits)

# ARICH reconstruction
# calculate PID likelihoods for all tracks
arichreco = register_module('ARICHReconstructor')
# use MC hits (ARICHAeroHits) instead of reconstructed tracks
arichreco.param('inputTrackType', 1)
# store Cherenkov angle information
arichreco.param('storePhotons', 1)
main.add_module(arichreco)

# ARICH Ntuple
# create flat ntuple for performance analysis
arichNtuple = register_module('ARICHNtuple')
arichNtuple.param('outputFile', options.filename)
main.add_module(arichNtuple)

# ARICH DQM
# create DQM occupancy plots
arichdqm = register_module('ARICHDQM')
main.add_module(arichdqm)

# Uncomment to store DataStore content to root file
# output = register_module('RootOutput')
# output.param('outputFileName', 'DataStore.root')
# main.add_module(output)

# Uncomment to show event display
# display = register_module('Display')
# display.param('showARICHHits', True)
# display.param('fullGeometry', True)
# main.add_module(display)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)

# Make basic performance plots
com = 'root -l ' + options.filename + ' ' + home + '/arich/utility/scripts/plotEfficiency.C'
os.system(com)
