#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from basf2 import *
from subprocess import call

numTracks = 1  # set number of tracks per event
numEvents = 100  # set number of events
seedValue = 1  # seed for rng
pMin = 0.05  # pT min
pMax = 0.5  # pT max
tMin = 52.  # theta min
tMax = 149.  # theta max

rSeed = seedValue

print 'Starting VXDTFModule example file, rSeed = ' + str(seedValue) \
    + ', number of events: ' + str(numEvents) + ' with ' + str(numTracks) \
    + ' tracks per event'
print 'INFO: Curling tracks are not supported yet.'
print 'INFO: There is a known problem with SVDClusters in slanted parts combined with the VXD track finder, until this is fixed, theta values in the range of 17°-50° will deliver poor results.'

set_log_level(LogLevel.ERROR)
set_random_seed(seedValue)

# register the modules and set there options
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [numEvents])

evtmetainfo = register_module('EvtMetaInfo')

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('Components', [
    'BeamPipe',
    'Cryostat',
    'HeavyMetalShield',
    'MagneticField',
    'PXD',
    'SVD',
    'SVD-Support',
    'CDC',
    ])

pGun = register_module('ParticleGun')
param_pGun = {  # 13: muons, 211: charged pions, 11 electrons
                # uniform
    'pdgCodes': [-13, 13],
    'nTracks': numTracks,
    'varyNTracks': False,
    'momentumGeneration': 'uniformPt',
    'momentumParams': [pMin, pMax],
    'thetaGeneration': 'uniform',
    'thetaParams': [tMin, tMax],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }
pGun.param(param_pGun)

g4sim = register_module('FullSim')

svdDigitizer = register_module('SVDDigitizer')
svdClusterizer = register_module('SVDClusterizer')

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.INFO
param_vxdtf = {  # #### tripple pass!
                 # minimal layer number allowed for TC-seeds
                 # minimal state allowed for TC-seeds. Higher numbers are more restrictive (better ghost rate), but remove good TCs as well
                 # -1 = VXD, 0 = PXD, 1 = SVD
                 # not working so far (too restrictive)
                 # not working at the moment
    'tccMinLayer': [5, 5, 4],
    'tccMinState': [2, 2, 1],
    'detectorType': [1, 1, 1],
    'sectorSetup': ['high', 'std', 'low'],
    'highestAllowedLayer': [6, 6, 5],
    'activateDistance3D': [True, True, True],
    'activateDistanceXY': [True, True, True],
    'activateDistanceZ': [True, True, True],
    'activateNormedDistance3D': [False, False, False],
    'activateDistanceDeltaZ': [False, False, False],
    'activateDistance2IP': [False, False, False],
    'activateAngles3D': [True, True, True],
    'activateAnglesXY': [True, True, True],
    'activateAnglesRZ': [True, True, True],
    'activateDeltaDistance2IP': [False, False, False],
    'activateZigZag': [True, True, True],
    'activateDeltaPt': [True, True, True],
    'qiSmear': True,
    'smearMean': 0.0,
    'smearSigma': 0.001,
    }
vxdtf.param(param_vxdtf)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(pGun)
main.add_module(g4sim)
main.add_module(svdDigitizer)
main.add_module(svdClusterizer)
main.add_module(vxdtf)
# Process events
process(main)

print statistics
