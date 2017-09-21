#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

# setup of the most important parts
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
numEvents = 25
initialValue = 42

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

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
telDigitizer = register_module('TelDigitizer')
telDigitizer.param('ElectronicNoise', 200)
telDigitizer.param('NoiseSN', 5.0)

pxdClusterizer = register_module('PXDClusterizer')
pxdClusterizer.param('Clusters', 'myPersonalPXDClusters')
svdClusterizer = register_module('SVDClusterizer')
svdClusterizer.param('Clusters', 'myPersonalSVDClusters')
telClusterizer = register_module('TelClusterizer')
telClusterizer.param('Clusters', 'myPersonalTelClusters')

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName',
              'testbeam/vxd/FullTelescopeVXDTB_v2_NOTAlignedAtAll.xml')

geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes the example much faster
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
    # secSetup = \
    # ['TB3GeVFullMagnetNoAlignedSource2014May22SVD-moreThan1500MeV_SVD']
    secSetup = ['testBeamMini6GeVJune08MagnetOnSVD-moreThan1500MeV_SVD']
    qiType = 'circleFit'  # circleFit
else:
    # To turn off magnetic field:
    geometry.param('components', ['TB'])
    # secSetup = ['TB4GeVNoMagnetNoAlignedSource2014May21SVD-moreThan1500MeV_SVD']
    secSetup = ['testBeamMini6GeVJune08MagnetOffSVD-moreThan1500MeV_SVD']
    qiType = 'straightLine'  # straightLine

particlegun = register_module('ParticleGun')
# number of primaries per event
particlegun.param('nTracks', 1)
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
g4sim.param('StoreAllSecondaries', True)

spCreatorSingle = register_module('SVDSpacePointCreator')
spCreatorSingle.logging.log_level = LogLevel.INFO
spCreatorSingle.logging.debug_level = 1
spCreatorSingle.param('OnlySingleClusterSpacePoints', True)
spCreatorSingle.param('NameOfInstance', 'singlesOnly')
spCreatorSingle.param('SpacePoints', 'singlesOnly')
spCreatorSingle.param('SVDClusters', 'myPersonalSVDClusters')

spCreatorCombi = register_module('SVDSpacePointCreator')
spCreatorCombi.logging.log_level = LogLevel.DEBUG
spCreatorCombi.logging.debug_level = 1
spCreatorCombi.param('OnlySingleClusterSpacePoints', False)
spCreatorCombi.param('NameOfInstance', 'couplesAllowed')
spCreatorCombi.param('SpacePoints', 'couplesAllowed')
spCreatorCombi.param('SVDClusters', 'myPersonalSVDClusters')

spCreatorPXD = register_module('PXDSpacePointCreator')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.logging.debug_level = 1
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
spCreatorPXD.param('SpacePoints', 'pxdOnly')
spCreatorPXD.param('PXDClusters', 'myPersonalPXDClusters')

spCreatorTEL = register_module('SpacePointCreatorTEL')
spCreatorTEL.logging.log_level = LogLevel.INFO
spCreatorTEL.logging.debug_level = 1
spCreatorTEL.param('NameOfInstance', 'telOnly')
spCreatorTEL.param('SpacePoints', 'telOnly')
spCreatorTEL.param('TelClusters', 'myPersonalTelClusters')

spCreatorTest = register_module('SpacePointCreatorTELTest')
spCreatorTest.logging.log_level = LogLevel.DEBUG
spCreatorTest.logging.debug_level = 50
spCreatorTest.param('NameOfInstance', 'SPTester')
spCreatorTest.param('SVDClusters', 'myPersonalSVDClusters')
spCreatorTest.param('PXDClusters', 'myPersonalPXDClusters')
spCreatorTest.param('TelClusters', 'myPersonalTelClusters')
spCreatorTest.param('AllSpacePointContainers', ['singlesOnly', 'couplesAllowed', 'pxdOnly', 'telOnly'])

# Create paths
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(telDigitizer)
main.add_module(telClusterizer)
main.add_module(spCreatorSingle)
main.add_module(pxdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(spCreatorCombi)
main.add_module(spCreatorTEL)
main.add_module(spCreatorPXD)
main.add_module(spCreatorTest)

# Process events
process(main)

print('Event Statistics :')
print(statistics)
