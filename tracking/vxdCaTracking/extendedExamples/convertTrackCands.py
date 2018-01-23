#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# example file invoking and testing the ConverterModules that convert genfit::TrackCands to SpacePointTrackCands and vice versa
import os
from basf2 import *
from sys import argv

numEvents = 100
numTracks = 4

# setting debuglevels for more than one module at once
MyLogLevel = LogLevel.INFO
MyDebugLevel = 500

initialValue = 0  # 0 for random events

if len(argv) is 1:
    print('no arguments given, using standard values')
if len(argv) is 2:
    initialValue = int(argv[1])
    print('using input value ' + str(initialValue) + ' as initialValue')

# important parameters for simulation
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

# module registration
set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB.xml')

geometry = register_module('Geometry')
geometry.param('components', ['TB'])

# ParticleGun
particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', numTracks)
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
particlegun.param('xVertexParams', [gun_x_position, 0.])
particlegun.param('yVertexParams', [0., beamspot_size_y])
particlegun.param('zVertexParams', [0., beamspot_size_z])
particlegun.param('independentVertices', True)

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # need for MCTrackfinder to work correctly

# digitizers & clusterizers
pxdDigitizer = register_module('PXDDigitizer')

# cant name clusters, because there is no way to pass these names to the TrackFinderMCTruth
pxdClusterizer = register_module('PXDClusterizer')
# pxdClusterizer.param('Clusters','myPXDClusters')

svdDigitizer = register_module('SVDDigitizer')
svdClusterizer = register_module('SVDClusterizer')
# svdClusterizer.param('Clusters','mySVDClusters')

# SpacePoint Creation
# single clusters only
spCreatorSVDsingle = register_module('SVDSpacePointCreator')
spCreatorSVDsingle.logging.log_level = LogLevel.INFO
spCreatorSVDsingle.param('OnlySingleClusterSpacePoints', True)
spCreatorSVDsingle.param('NameOfInstance', 'singleClusters')
spCreatorSVDsingle.param('SpacePoints', 'singleSP')
# spCreatorSVD.param('SVDClusters', 'singleSP')

# double clusters
spCreatorSVD = register_module('SVDSpacePointCreator')
spCreatorSVD.logging.log_level = LogLevel.INFO
param_spCreatorSVD = {'OnlySingleClusterSpacePoints': False,
                      'NameOfInstance': 'couplesClusters',
                      'SpacePoints': 'nosingleSP'}
spCreatorSVD.param(param_spCreatorSVD)

spCreatorPXD = register_module('PXDSpacePointCreator')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
# spCreatorPXD.param('PXDClusters', 'myPXDClusters')
spCreatorPXD.param('SpacePoints', 'pxdOnly')

# MCTrackFinder
mcTrackFinder = register_module('TrackFinderMCTruth')
mcTrackFinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 5,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
}
mcTrackFinder.param(param_mctrackfinder)

# TCConverter, genfit -> SPTC
trackCandConverter = register_module('GFTC2SPTCConverter')
trackCandConverter.logging.log_level = MyLogLevel
trackCandConverter.logging.debug_level = MyDebugLevel
param_trackCandConverter = {  # 'PXDClusters': 'myPXDClusters',
                              #    'SVDClusters': 'mySVDClusters',
    'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracks',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'SingleClusterSVDSP': 'singleSP',
    'PXDClusterSP': 'pxdOnly',
}
trackCandConverter.param(param_trackCandConverter)

# TCConverter, SPTC -> genfit
btrackCandConverter = register_module('SPTC2GFTCConverter')
btrackCandConverter.logging.log_level = MyLogLevel
btrackCandConverter.logging.debug_level = MyDebugLevel
param_btrackCandConverter = {'SpacePointTCName': 'SPTracks',
                             'genfitTCName': 'myMCTracks'}
btrackCandConverter.param(param_btrackCandConverter)

# TCConverterTest
tcConverterTest = register_module('TCConvertersTest')
tcConverterTest.logging.log_level = MyLogLevel
tcConverterTest.logging.debug_level = MyDebugLevel
param_tcConverterTest = {'SpacePointTCName': 'SPTracks',
                         'genfitTCNames': ['mcTracks', 'myMCTracks'],
                         'SpacePointArrayNames': 'pxdOnly'}
tcConverterTest.param(param_tcConverterTest)

# Path
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)
main.add_module(pxdDigitizer)
main.add_module(svdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdClusterizer)
main.add_module(spCreatorPXD)
main.add_module(spCreatorSVD)
main.add_module(spCreatorSVDsingle)
main.add_module(mcTrackFinder)

main.add_module(trackCandConverter)
main.add_module(btrackCandConverter)
main.add_module(tcConverterTest)

process(main)

print(statistics)
