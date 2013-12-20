#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Common PXD&SVD TestBeam Jan 2014 @ DESY Simulation
# This is the default simulation scenario for VXD beam test without telescopes

# Important parameters of the simulation:
events = 4000  # Number of events to simulate
momentum = 6.  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.00573  # # degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.00573  # degrees (sigma of gaussian)
gun_x_position = 750.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.05  # cm (sigma of gaussian)
beamspot_size_z = 0.05  # cm (sigma of gaussian)

from basf2 import *
# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)
# ParticleGun
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', 10)
# DESY electrons:
particlegun.param('pdgCodes', [11])
# momentum magnitude 2 GeV/c or something above or around.
# At DESY we can have up to 6 GeV/c(+-5%) electron beam.
# Beam divergence divergence and spot size is adjusted similar to reality
# See studies of Benjamin Schwenker
particlegun.param('momentumGeneration', 'normal')
particlegun.param('momentumParams', [momentum, momentum * momentum_spread])
# momentum direction must be around theta=90, phi=180
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
# gun position must be in positive values of x.
# Magnet wall starts at 424mm and ends at 590mm
# Plastic 1cm shielding is at 650mm
# Aluminium target at 750mm to "simulate" 15m air between collimator and TB setup
particlegun.param('vertexGeneration', 'normal')
particlegun.param('xVertexParams', [gun_x_position, 0.0])
particlegun.param('yVertexParams', [0.0, beamspot_size_y])
particlegun.param('zVertexParams', [0.0, beamspot_size_z])
particlegun.param('independentVertices', True)

# Create Event information
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [events], 'runList': [1]})

# Show progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# This file contains the VXD (no Telescopes) beam test geometry including the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/BeamLine.xml')

# Create geometry
geometry = register_module('Geometry')
# You can specify components to be created
geometry.param('Components', ['TB'])
# To turn off magnetic field:
# geometry.param('Components', ['TB'])

# Full simulation module
simulation = register_module('FullSim')

# Uncomment the following lines to get particle tracks visualization
# simulation.param('EnableVisualization', True)
# simulation.param('UICommands', ['/vis/open VRML2FILE', '/vis/drawVolume',
#                 '/vis/scene/add/axes 0 0 0 100 mm',
#                 '/vis/scene/add/trajectories smooth',
#                 '/vis/modeling/trajectories/create/drawByCharge'])

# PXD/SVD digitizer
PXDDigi = register_module('PXDDigitizer')
SVDDigi = register_module('SVDDigitizer')

# This module changes specified PXD sensors to binary readout: EUDET emulation
# It must be before PXDClusterizer in the path!
EUDET_emulator = register_module('ChangePXDDigitsToBinary')

# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
# ClusterCacheSize must be changed to minimum 576 (number of columns) for EUDET matrices (576x1152) to work.
# Otherwise you will get 'segmentation violation'.
PXDClust.param('ClusterCacheSize', 576)

SVDClust = register_module('SVDClusterizer')

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'BeamScanSimulationOutputWien.root')
# You can specify branch names to be saved (only), see module doc
# output.param('branchNames[0]', ['PXDTrueHits', 'SVDTrueHits'])

# Export used geometry for checking
geosaver = register_module('ExportGeometry')
geosaver.param('Filename', 'TBGeometry.root')

# Use truth information to create track candidates
# mctrackfinder = register_module('TrackFinderMCTruth')
# mctrackfinder.logging.log_level = LogLevel.WARNING
# param_mctrackfinder = {
#    'UseCDCHits': 0,
#    'UseSVDHits': 1,
#    'UsePXDHits': 1,
#    'Smearing': 0,
#    'UseClusters': True,
#    }
# mctrackfinder.param(param_mctrackfinder)

# mctrackfinder.logging.log_level = LogLevel.DEBUG

# Fit tracks with GENFIT
# trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
# trackfitter.param('UseClusters', True)

# Check track fitting results
# trackfitchecker = register_module('TrackFitChecker')
# the results only show up at info or debug level
# trackfitchecker.logging.log_level = LogLevel.INFO
# trackfitchecker.param('inspectTracks', True)
# trackfitchecker.param('truthAvailable', True)
# trackfitchecker.param('testSi', True)
# trackfitchecker.param('robustTests', True)
# trackfitchecker.param('writeToRootFile', True)

# display = register_module('Display')

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
# display = register_module('Display')

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
# display.param('showAllPrimaries', True)

# show all charged MCParticles? (SLOW)
# display.param('showCharged', False)

# show tracks?
# display.param('showTrackLevelObjects', True)

# save events non-interactively (without showing window)?
# display.param('automatic', False)

# Use clusters to display tracks
# display.param('useClusters', False)

# Display the testbeam geometry rather than Belle II extract
# display.param('fullGeometry', True)

# Path construction
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(simulation)

histoman = register_module('HistoManager')
histoman.param('histoFileName', 'beamscanwien.root')
main.add_module(histoman)

beamscan = register_module('BeamspotScan')
beamscan.param('nBinX', 24)
beamscan.param('nBinY', 24)
beamscan.param('minX', -1.2)
beamscan.param('minY', -1.2)
beamscan.param('maxX', 1.2)
beamscan.param('maxY', 1.2)
main.add_module(beamscan)

# main.add_module(PXDDigi)
# main.add_module(EUDET_emulator)
# main.add_module(SVDDigi)
# main.add_module(PXDClust)
# main.add_module(SVDClust)
# main.add_module(mctrackfinder)
# main.add_module(trackfitter)
# main.add_module(trackfitchecker)
main.add_module(geosaver)
main.add_module(output)
# main.add_module(display)
# Process events
process(main)

# Print call statistics
print statistics
