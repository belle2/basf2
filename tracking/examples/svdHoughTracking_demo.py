#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Steering file to test tracking code
#
import os
import sys
import getopt
from basf2 import *
from subprocess import call

# Options
events = 100  # Number of events to simulate
fieldOn = True  # Turn magnet field on or off
particleGun = True  # Particle gun on or off?
bgMixer = True  # background mixer
random_dir = True  # Random angles
fixedTheta = True  # Fix Theta, but can be uniform distributed with min_ and max_theta
fixedPhi = False  # Fix Phi
displayOn = False  # Enable display
tb_rectsize = False  # Tb rectsize

# Check if debug directory exists
if not os.path.isdir('dbg'):
    os.makedirs('dbg')

argv = sys.argv[1:]
try:
    (opts, args) = getopt.getopt(argv, 'hbdf:')
except getopt.GetoptError:
    print 'test.py -h for help'
    sys.exit(2)
for (opt, arg) in opts:
    if opt == '-h':
        print '<cmd> read the source, Luke...\n'
        sys.exit(2)
    elif opt in '-d':
        print 'Enable display\n'
        displayOn = True
    elif opt in '-b':
        print 'Disable bkg\n'
        bgMixer = False
    elif opt in '-f':
        input_file = arg
        print 'Input file is: ' + input_file + '\n'

# Track parameters
particle_ids = [-11, 11]  # particles
momentum = 3.000  # GeV/c
momentum_spread = 0.000  # %
momUniform = True
min_mom = 0.05
max_mom = 2.00
n_tracks = 10  # number of tracks
theta = 90.0
theta_spread = 20.000  # degrees (sigma of gaussian)
thetaUniform = True
min_theta = 17.0
max_theta = 150.0
phi = 55.0  # degrees
phi_spread = 0.000  # degrees (sigma of gaussian)
gun_x_position = 0.000  # cm ... 40cm ... inside magnet
beamspot_size_x = 0.01  # cm (sigma of gaussian)
beamspot_size_y = 0.06  # cm (sigma of gaussian)
beamspot_size_z = 0.4  # cm (sigma of gaussian)

# Background dir
bkgDir = '/data/schnell/background/09.06.2014/'

# Log level
set_log_level(LogLevel.ERROR)

# Set seed
# set_random_seed(10383309)
set_random_seed(3)

evtmetagen = register_module('EventInfoSetter')
evtmetagen.param('expList', [0])
evtmetagen.param('runList', [1])
evtmetagen.param('evtNumList', [events])

# Load parameters from xml
gearbox = register_module('Gearbox')

#
# Geometry
#
geometry = register_module('Geometry')

# With or without magnet field?
if fieldOn:
    geometry.param('components', ['MagneticField', 'PXD', 'SVD'])
else:
    geometry.param('components', ['PXD', 'SVD'])

#
# ParticleGun
#
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', n_tracks)
particlegun.param('pdgCodes', particle_ids)
if momUniform:
    particlegun.param('momentumGeneration', 'uniform')
    particlegun.param('momentumParams', [min_mom, max_mom])
else:
    particlegun.param('momentumGeneration', 'normal')
    particlegun.param('momentumParams', [momentum, momentum * momentum_spread])

# Direction
if random_dir:
    if fixedTheta:
        if thetaUniform:
            particlegun.param('thetaGeneration', 'uniform')
            particlegun.param('thetaParams', [min_theta, max_theta])
        else:
            particlegun.param('thetaGeneration', 'normal')
            particlegun.param('thetaParams', [theta, theta_spread])
    else:
        particlegun.param('thetaGeneration', 'normal')
    if fixedPhi:
        particlegun.param('phiGeneration', 'fixed')
        particlegun.param('phiParams', [phi, phi_spread])
    else:
        particlegun.param('phiGeneration', 'normal')
else:
    particlegun.param('thetaGeneration', 'fixed')
    particlegun.param('thetaParams', [theta, theta_spread])
    particlegun.param('phiGeneration', 'normal')
    particlegun.param('phiParams', [phi, phi_spread])

# Vertex
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, beamspot_size_x])
particlegun.param('yVertexParams', [0.000, beamspot_size_y])
particlegun.param('zVertexParams', [0.000, beamspot_size_z])
particlegun.param('independentVertices', True)

#
# Full sim
#
g4sim = register_module('FullSim')

# this is needed for the MCTrackFinder to work correctly
g4sim.param('StoreAllSecondaries', True)

#
# Background mixer
#
bgmixer = register_module('BeamBkgMixer')
# bgmixer = register_module('MixBkg') # Old bkg mixer

# ROF files
bgmixer.param('backgroundFiles', [
    bkgDir + 'Coulomb_HER_100us.root',
    bkgDir + 'Coulomb_LER_100us.root',
    bkgDir + 'RBB_HER_100us.root',
    bkgDir + 'RBB_LER_100us.root',
    bkgDir + 'Touschek_HER_100us.root',
    bkgDir + 'Touschek_LER_100us.root',
    bkgDir + 'twoPhoton_200us.root',
    ])

# Options
# bgmixer.param('AnalysisMode', False)

# Set time scale to something realistic in the svd
bgmixer.param('minTime', -4)
bgmixer.param('maxTime', 4)
bgmixer.param('components', ['PXD', 'SVD'])  # Included components
bgmixer.set_log_level(LogLevel.INFO)
bgmixer.logging.debug_level = 0

#
# SVD digitizer
#
SVDDIGI = register_module('SVDDigitizer')

# Noise and electronics effects
SVDDIGI.param('PoissonSmearing', False)
SVDDIGI.param('ElectronicEffects', False)
# SVDDIGI.logging.log_level = LogLevel.DEBUG

#
# SVD Clusterizer
#
SVDCLUST = register_module('SVDClusterizer')
# SVDCLUST.logging.log_level = LogLevel.DEBUG

#
# PXD digitizer
#
PXDDIGI = register_module('PXDDigitizer')

# Noise and electronics effects
PXDDIGI.param('SimpleDriftModel', False)
PXDDIGI.param('PoissonSmearing', True)
PXDDIGI.param('ElectronicEffects', True)
# PXDDIGI.logging.log_level = LogLevel.DEBUG

#
# PXD clusterizer
#
PXDCLUST = register_module('PXDClusterizer')

#
# SVD Hough tracking
#
SVDHough = register_module('SVDHoughtracking')

# Hough tracking
# SVDHough.param('SVDHoughCluster', 'hough_cluster');
# SVDHough.param('SVDHoughTrack', 'hough_tracks');
SVDHough.param('PXDExtrapolatedHits', 'extrapolated_hits')

# Clustering
SVDHough.param('UseClusters', False)
SVDHough.param('UseFPGAClusters', False)
SVDHough.param('DisableNoiseFilter', True)
SVDHough.param('AnalyseFPGAClusters', False)
if bgMixer:
    SVDHough.param('UseTrueHitClusters', False)
    SVDHough.param('UseSimHitClusters', True)
else:
    SVDHough.param('UseTrueHitClusters', True)
    SVDHough.param('UseSimHitClusters', False)

# Tracking
SVDHough.param('FullTrackingPipeline', True)
SVDHough.param('TbMapping', False)  # Testbeam mapping!
SVDHough.param('StraightTracks', False)  # Use Straight tracks only for testbeam
SVDHough.param('ConformalTrafoN', False)
SVDHough.param('ConformalTrafoP', True)
SVDHough.param('WriteHoughSpace', True)
SVDHough.param('WriteHoughSectors', True)
SVDHough.param('UsePhiOnly', False)
SVDHough.param('UseSensorFilter', True)
SVDHough.param('UseHashPurify', False)
SVDHough.param('minimumLines', 3)
SVDHough.param('criticalIterationsN', 6)
SVDHough.param('criticalIterationsP', 7)

SVDHough.param('rectSizeN', 2.00)
SVDHough.param('rectSizeP', 0.05)
SVDHough.param('rectScaleN', 1.0)
SVDHough.param('rectScaleP', 1.0)
if tb_rectsize:
    SVDHough.param('rectXP1', 1.0)
    SVDHough.param('rectXP2', 2.00)
    SVDHough.param('rectXN1', 1.0)
    SVDHough.param('rectXN2', 2.00)

SVDHough.param('RadiusThreshold', 10.0)

SVDHough.param('CompareWithMCParticle', True)
SVDHough.param('CompareWithMCParticleVerbose', True)

SVDHough.param('UseTrackMerger', True)
SVDHough.param('MergeThreshold', 0.01)

# Extrapolation
SVDHough.param('PXDExtrapolation', False)
SVDHough.param('PXDTbExtrapolation', False)
SVDHough.param('UsePhiExtrapolation', False)
SVDHough.param('UseThetaExtrapolation', False)
# SVDHough.param('SVDSimHits', 'SVDSimHits');
# SVDHough.param('SVDPosName', 'position.dat');
# SVDHough.param('SVDStripName', 'strips.dat');

# Histograms and statistic file
SVDHough.param('statisticsFilename', 'statistics.dat')
SVDHough.param('rootFilename', 'hists.root')

# Debug stuff
SVDHough.param('PrintTrackInfo', False)
SVDHough.param('PrintStatistics', True)
SVDHough.logging.log_level = LogLevel.DEBUG
SVDHough.logging.debug_level = 0

#
# Display
#
display = register_module('Display')

# The Options parameter is a combination of:
# D draw detectors - draw simple detector representation (with different size)
#   for each hit
# H draw track hits
# M draw track markers - intersections of track with detector planes
#   (use with T)
# P draw detector planes
# S scale manually - spacepoint hits are drawn as spheres and scaled with
#   errors
# T draw track (straight line between detector planes)
#
# Note that you can always turn off an individual detector component or track
# interactively by removing its checkmark in the 'Eve' tab.
#
# This option only makes sense when ShowGFTracks is true
# display.param('options', 'HTM')  # default

# should hits always be assigned to a particle with c_PrimaryParticle flag?
# with this option off, many tracking hits will be assigned to secondary e-
# display.param('assignHitsToPrimaries', 0)

# show all primary MCParticles?
display.param('showAllPrimaries', True)

# show all charged MCParticles? (SLOW)
# display.param('showCharged', False)

# save events non-interactively (without showing window)?
# display.param('automatic', False)

# Use clusters to display tracks
display.param('useClusters', True)

# Display the testbeam geometry rather than Belle II extract
display.param('fullGeometry', True)

# SVD Hough parameters
# display.param('SVDHoughCluster', 'hough_cluster');
# display.param('SVDHoughTrack', 'hough_tracks');
# display.param('PXDExtrapolatedHits', 'extrapolated_hits');
display.logging.log_level = LogLevel.INFO

# Progress module
progress = register_module('Progress')

#
# Root output module
#
output = register_module('RootOutput')

# Set output filename
output.param('outputFileName', 'fullRootOutput.root')

# Create paths
main = create_path()

main.add_module(evtmetagen)
# main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
if particleGun:
    main.add_module(particlegun)
main.add_module(g4sim)
if bgMixer:
    main.add_module(bgmixer)
main.add_module(PXDDIGI)
main.add_module(PXDCLUST)
main.add_module(SVDDIGI)
main.add_module(SVDCLUST)
main.add_module(SVDHough)
main.add_module(progress)
if displayOn:
    main.add_module(display)

# Process events
process(main)

print statistics
