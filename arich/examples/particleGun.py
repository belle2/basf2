#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from optparse import OptionParser
from tracking import add_tracking_reconstruction

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
geometry = register_module('Geometry')
geometry.param('components', [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'ARICH'])
main.add_module(geometry)

# Particle gun: generate multiple tracks
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

# ARICH digitization
arichDigi = register_module('ARICHDigitizer')
main.add_module(arichDigi)

# tracking
add_tracking_reconstruction(main)

# Track extrapolation
ext = register_module('Ext')
main.add_module(ext)

# convert ARICHDigits to ARICHHits
arichHits = register_module('ARICHFillHits')
main.add_module(arichHits)

# ARICH reconstruction
# calculate PID likelihoods for all tracks
arichreco = register_module('ARICHReconstructor')
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
