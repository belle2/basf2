#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

# Important parameters of the simulation:
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # # degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

numTracks = 1
numEvents = 25000
initialValue = 0  # want random events

# parameters for the secMap-calculation:
pTcuts = [1.0, 1.5]

# setupFileName = "testBeamMini"
# secConfigU = [0.0, 1.0]
# secConfigV = [0.0, 1.0]

# setupFileName = "testBeamStd"
# secConfigU = [0.0, 0.5, 1.0]
# secConfigV = [0.0, 0.33, 0.67, 1.0]

setupFileName = 'testBeamFine'
secConfigU = [0., 0.25, 0.5, 0.75, 1.0]
secConfigV = [
    0.,
    0.2,
    0.4,
    0.6,
    0.8,
    1.0,
    ]

# allows steering initial value and numEvents by sript file
if len(argv) is 1:
    print ' no arguments given, using standard values'
elif len(argv) is 2:
    initialValue = int(argv[1])
else:
    initialValue = int(argv[2])
    numEvents = int(argv[1])

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('expList', [0])
evtmetagen.param('runList', [1])
evtmetagen.param('evtNumList', [numEvents])
evtmetainfo = register_module('EvtMetaInfo')

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName', 'testbeam/vxd/VXD-simple-noTels-30Oct13.xml')

geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes this example much faster
geometry.param('Components', ['MagneticField', 'TB'])

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
# this is needed for the MCTrackFinder to work correctly
g4sim.param('StoreAllSecondaries', True)

print ''
print 'entering createSecMapForVXDTF.py'
print 'starting {events:} events, analyzing {numTracks:} track(s) per event by using pGseed {theSeed:}. '.format(events=numEvents,
        numTracks=numTracks, theSeed=initialValue)
print ''

setupFileNamevxd = setupFileName + 'VXD'
filterCalc = register_module('FilterCalculator')
filterCalc.logging.log_level = LogLevel.DEBUG
filterCalc.logging.debug_level = 1
param_fCalc = {  # -1 = VXD, 0 = PXD, 1 = SVD
                 # # completely different to values of Belle2-detector
    'detectorType': -1,
    'maxXYvertexDistance': 200.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 0,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigU,
    'sectorConfigV': secConfigV,
    'setOrigin': [gun_x_position, 0., 0.],
    'magneticFieldStrength': 0.976,
    'testBeam': 1,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamevxd,
    'smearHits': 1,
    'uniSigma': 0.3,
    'noCurler': 1,
    }
filterCalc.param(param_fCalc)

setupFileNamesvd = setupFileName + 'SVD'
filterCalc2 = register_module('FilterCalculator')
filterCalc2.logging.log_level = LogLevel.DEBUG
filterCalc2.logging.debug_level = 1
param_fCalc2 = {  # -1 = VXD, 0 = PXD, 1 = SVD
                  # # completely different to values of Belle2-detector
    'detectorType': 1,
    'maxXYvertexDistance': 200.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 0,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigU,
    'sectorConfigV': secConfigV,
    'setOrigin': [gun_x_position, 0., 0.],
    'magneticFieldStrength': 0.976,
    'testBeam': 1,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamesvd,
    'smearHits': 1,
    'uniSigma': 0.3,
    'noCurler': 1,
    }
filterCalc2.param(param_fCalc2)

# using one export module only
exportXML = register_module('ExportSectorMap')
exportXML.logging.log_level = LogLevel.DEBUG
exportXML.logging.debug_level = 30
exportXML.param('rootFileName', setupFileName)

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 250)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)

main.add_module(eventCounter)
main.add_module(filterCalc)
main.add_module(filterCalc2)

main.add_module(exportXML)
# Process events
process(main)

print statistics
