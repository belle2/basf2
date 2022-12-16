#!/usr/bin/env python3

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

# --------------------------------------------------------------------
# Example script for ARICH stand alone simulation and reconstruction
# (only ARICH is simulated, very fast)
# Instead of tracks from CDC, the track information is taken from MC hits
# on aerogel plane (ARICHAeroHit).
# Smearing of track position and direction is applied to mimic tracking
# resolution.
# --------------------------------------------------------------------

parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=1000, help='Number of events to process')
parser.add_option('-f', '--file', dest='filename', default='ARICHEvents.root')
parser.add_option('-r', '--hepr', action="store_true", dest='hepr', default=False, help='Visualisation with heprep')
parser.add_option('-o', '--overlap', action="store_true", dest='overlap', default=False, help='Run overlap checker')
parser.add_option('-b', '--rootbatch', action="store_true", dest='rootbatch',
                  default=False, help='Run analysis root script in batch mode')
parser.add_option('-m', '--rootoff', action="store_true", dest='rootoff', default=False, help='Do not run root script in the end')

(options, args) = parser.parse_args()

home = os.environ['BELLE2_LOCAL_DIR']

# set specific database tag
# b2.conditions.override_globaltags(["tagname"])
# use local database
# b2.conditions.testing_payloads = ["localdb/database.txt"]

# Suppress messages and warnings during processing:
b2.set_log_level(b2.LogLevel.ERROR)

# Create path
main = b2.create_path()

# Set number of events to generate
eventinfosetter = b2.register_module('EventInfoSetter')
if not (options.hepr):
    if not (options.overlap):
        eventinfosetter.param({'evtNumList': [int(options.nevents)], 'runList': [1]})
if (options.hepr):
    eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
if (options.overlap):
    eventinfosetter.param({'evtNumList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', 'DQMhistograms.root')  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
main.add_module('Gearbox')

# Geometry
# only ARICH and magnetic field
geometry = b2.register_module('Geometry')
geometry.param('components', [
    'MagneticField',
    'ARICH'])
main.add_module(geometry)

# Particle gun
particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211, 321, -321])
particlegun.param('nTracks', 1)
# particlegun.param('varyNTracks', True)
particlegun.param('momentumGeneration', 'uniform')
particlegun.param('momentumParams', [0.5, 4])
# particlegun.param('momentumParams', [0.4, 0.8])
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
simulation = b2.register_module('FullSim')
# Visualisation with HepRep
if (options.hepr):
    print('Visualisation with HepRep')
    simulation.param('EnableVisualization', True)
    simulation.param('UICommandsAtIdle', [
        '/vis/open HepRepFile',
        '/vis/scene/create',
        '/vis/scene/add/volume',
        '/vis/sceneHandler/attach',
        '/vis/viewer/flush',
        '/vis/scene/add/trajectories smooth',
        '/vis/scene/add/hits'
    ])

main.add_module(simulation)

# Check for volume intersection/overlaps
if (options.overlap):
    print('Check for volume intersection/overlaps')
    overlapchecker = b2.register_module('OverlapChecker')
    main.add_module(overlapchecker)

# ARICH digitization
arichDigi = b2.register_module('ARICHDigitizer')
main.add_module(arichDigi)

# convert ARICHDigits to ARICHHits
arichHits = b2.register_module('ARICHFillHits')
main.add_module(arichHits)

# ARICH reconstruction
# calculate PID likelihoods for all tracks
arichreco = b2.register_module('ARICHReconstructor')
# use MC hits (ARICHAeroHits) instead of reconstructed tracks
arichreco.param('inputTrackType', 1)
# store Cherenkov angle information
arichreco.param('storePhotons', 1)
main.add_module(arichreco)

# ARICH Ntuple
# create flat ntuple for performance analysis
arichNtuple = b2.register_module('ARICHNtuple')
arichNtuple.param('outputFile', options.filename)
main.add_module(arichNtuple)

# ARICH DQM
# create DQM occupancy plots
arichdqm = b2.register_module('ARICHDQM')
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
main.add_module('Progress')

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)

# Make basic performance plots
if (options.rootbatch):
    com = 'root -l -b -q ' + options.filename + ' ' + home + '/arich/utility/scripts/plotEfficiency.C'
else:
    com = 'root -l ' + options.filename + ' ' + home + '/arich/utility/scripts/plotEfficiency.C'

if not (options.hepr or options.overlap or options.rootoff):
    os.system(com)

# Retrieve of the histograms from TCanvas produced by plotEfficiency.C
com = 'root -l -b -q ' + 'ARICHPerformance.root' + ' ' + home + '/arich/utility/scripts/plotEfficiencyConvertTCanvasToHist.C'
if not (options.hepr or options.overlap or options.rootoff):
    os.system(com)
