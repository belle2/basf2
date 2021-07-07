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
from tracking import add_tracking_reconstruction
from simulation import add_simulation
import os
# --------------------------------------------------------------------
# Example of using ARICH reconstruction
# needs reconstructed tracks (Tracks), extrapolated to ARICH (ExtHits)
# --------------------------------------------------------------------

parser = OptionParser()
parser.add_option('-n', '--nevents', dest='nevents', default=1000,
                  help='Number of events to process')
parser.add_option('-f', '--file', dest='filename',
                  default='ARICHEvents.root')
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
eventinfosetter.param({'evtNumList': [int(options.nevents)], 'runList': [1]})
main.add_module(eventinfosetter)

# Histogram manager immediately after master module
histo = b2.register_module('HistoManager')
histo.param('histoFileName', 'DQMhistograms.root')  # File to save histograms
main.add_module(histo)

# Gearbox: access to database (xml files)
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Particle gun: generate multiple tracks
particlegun = b2.register_module('ParticleGun')
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

# Simulation & Digitizer of inner detectors
add_simulation(main, usePXDDataReduction=False)
# tracking
add_tracking_reconstruction(main)

# Track extrapolation
ext = b2.register_module('Ext')
main.add_module(ext)

# convert ARICHDigits to ARICHHits
arichHits = b2.register_module('ARICHFillHits')
main.add_module(arichHits)

# ARICH reconstruction
# calculate PID likelihoods for all tracks
arichreco = b2.register_module('ARICHReconstructor')
# store cherenkov angle information
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
progress = b2.register_module('Progress')
main.add_module(progress)

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)

# Make basic performance plots
com = 'root -l ' + options.filename + ' ' + home + '/arich/utility/scripts/plotEfficiency.C'
os.system(com)
