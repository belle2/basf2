#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

# Important parameters of the simulation:
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  # degrees (sigma of gaussian)
phi = 0.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = -100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)

numTracks = 1
numEvents = 15000
initialValue = 0  # want random events
usePXD = True
checkTH = True

fieldValue = 0.  # expected magnetic field in Tesla
# parameters for the secMap-calculation:
pTcuts = [1.5]
setupFileName = 'TB2016Test8Feb2016'

if fieldOn:
    fieldValue = 1.
    setupFileName += 'MagnetOn'
else:
    setupFileName += 'MagnetOff'

secConfigU = [0., 1.]
secConfigV = [0., 1.]
setupFileNamesvd = setupFileName + 'SVD'
setupFileNamevxd = setupFileName + 'PXDSVD'

# setupFileName = "testBeamStd"
# secConfigU = [0.0, 0.5, 1.0]
# secConfigV = [0.0, 0.33, 0.67, 1.0]

# setupFileName = 'testBeamFine'
# secConfigU = [0., 0.25, 0.5, 0.75, 1.0]
# secConfigV = [
# 0.,
# 0.2,
# 0.4,
# 0.6,
# 0.8,
# 1.0,
# ]

# allows steering initial value and numEvents by sript file
if len(argv) is 1:
    print(' no arguments given, using standard values')
elif len(argv) is 2:
    initialValue = int(argv[1])
else:
    initialValue = int(argv[2])
    numEvents = int(argv[1])

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB2016.xml')

geometry = register_module('Geometry')
# You can specify components to be created
if fieldOn:
    print("nothing to exclude")
    # geometry.param('excludedComponents', [''])
else:
    # To turn off magnetic field:
    geometry.param('excludedComponents', ['MagneticField'])

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
# momentum direction must be around theta=90, phi=0
particlegun.param('thetaGeneration', 'normal')
particlegun.param('thetaParams', [theta, theta_spread])
particlegun.param('phiGeneration', 'normal')
particlegun.param('phiParams', [phi, phi_spread])
# gun position must be in negative values of x.
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

# Show progress of processing
progress = register_module('Progress')

print('')
print('entering createSecMapForVXDTF.py')
print(
    'starting {events:} events, analyzing {numTracks:} track(s) per event by using pGseed {theSeed:}. '.format(
        events=numEvents,
        numTracks=numTracks,
        theSeed=initialValue))
print('')

filterCalc = register_module('SecMapTrainerWithSpacePoints')
# filterCalc = register_module('FilterCalculator')
filterCalc.logging.log_level = LogLevel.INFO
filterCalc.logging.debug_level = 10
param_fCalc = {
    'detectorType': ['PXD', 'SVD'],
    'acceptedRegionForSensors': [-1, -1],
    'maxXYvertexDistance': 200.,
    'maxZvertexDistance': 200,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigU,
    'sectorConfigV': secConfigV,
    'setOrigin': [gun_x_position * 0.4, 0., 0.],
    'magneticFieldStrength': fieldValue,
    'testBeam': 2,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamevxd,
    'smearHits': 1,
    'uniSigma': 0.3,
    'noCurler': 1,
    'spTCarrayName': 'checkedSPTCs',
}
filterCalc.param(param_fCalc)

filterCalc2 = register_module('SecMapTrainerWithSpacePoints')
filterCalc2.logging.log_level = LogLevel.INFO
filterCalc2.logging.debug_level = 100
param_fCalc2 = {
    'detectorType': ['SVD'],
    'acceptedRegionForSensors': [-1, -1],
    'maxXYvertexDistance': 200.,
    'maxZvertexDistance': 200,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigU,
    'sectorConfigV': secConfigV,
    'setOrigin': [gun_x_position * 0.4, 0., 0.],
    'magneticFieldStrength': fieldValue,
    'testBeam': 2,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamesvd,
    'smearHits': 1,
    'uniSigma': 0.3,
    'noCurler': 1,
    'spTCarrayName': 'checkedSPTCs',
}
filterCalc2.param(param_fCalc2)


# using one export module only
exportXML = register_module('RawSecMapMergerWithSpacePoints')
exportXML.logging.log_level = LogLevel.DEBUG
exportXML.logging.debug_level = 10
exportXML.param('rootFileName', setupFileName)
exportXML.param('sortByDistance2origin', True)


doPXD = 0
if usePXD:
    doPXD = 1
mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': doPXD,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
}
mctrackfinder.param(param_mctrackfinder)

spCreatorSVD = register_module('SVDSpacePointCreator')
spCreatorSVD.logging.log_level = LogLevel.INFO
spCreatorSVD.logging.debug_level = 5
param_spCreatorSVD = {'OnlySingleClusterSpacePoints': False,
                      'NameOfInstance': 'couplesClusters',
                      'SpacePoints': 'nosingleSP'}
spCreatorSVD.param(param_spCreatorSVD)

spCreatorPXD = register_module('PXDSpacePointCreator')
spCreatorPXD.logging.log_level = LogLevel.INFO
spCreatorPXD.logging.debug_level = 5
spCreatorPXD.param('NameOfInstance', 'pxdOnly')
spCreatorPXD.param('SpacePoints', 'pxdOnly')


sp2thConnector = register_module('SpacePoint2TrueHitConnector')
sp2thConnector.logging.log_level = LogLevel.WARNING
param_sp2thConnector = {
    'DetectorTypes': ['PXD', 'SVD'],
    'TrueHitNames': ['', ''],
    'ClusterNames': ['', ''],
    'SpacePointNames': ['pxdOnly', 'nosingleSP'],
    'outputSuffix': '_relTH',
    'storeSeperate': False,
    'registerAll': False,
    'maxGlobalPosDiff': 0.05,
    'maxPosSigma': 5,
    'minWeight': 0,
    'requirePrimary': True,
    'positionAnalysis': False,
}
sp2thConnector.param(param_sp2thConnector)


# TCConverter, genfit -> SPTC
trackCandConverter = register_module('GFTC2SPTCConverter')
trackCandConverter.logging.log_level = LogLevel.INFO
param_trackCandConverterReference = {
    'genfitTCName': 'mcTracks',
    'SpacePointTCName': 'SPTracks',
    'NoSingleClusterSVDSP': 'nosingleSP',
    'PXDClusterSP': 'pxdOnly',
    'checkTrueHits': checkTH,
    'useSingleClusterSP': False,
    'checkNoSingleSVDSP': True,
}
trackCandConverter.param(param_trackCandConverterReference)


# SpacePointTrackCand referee
sptcReferee = register_module('SPTCReferee')
sptcReferee.logging.log_level = LogLevel.INFO
param_sptcReferee = {
    'sptcName': 'SPTracks',
    'newArrayName': 'checkedSPTCs',
    'storeNewArray': True,
    'checkCurling': False,
    'splitCurlers': False,
    'keepOnlyFirstPart': True,
    'kickSpacePoint': True,  # not necessarily essential -> alternatve: check and filter in SecMapTrainerBase
    'checkSameSensor': True,
    'useMCInfo': True,
}
sptcReferee.param(param_sptcReferee)


eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 250)

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')

# testing
# svdClusterizer.param('Clusters','dummyName');


log_to_file('createSecMapForVXDTF/events_' + str(initialValue) + '/' + str(numEvents) + '.txt', append=False)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
main.add_module(g4sim)

main.add_module(pxdDigitizer)
main.add_module(svdDigitizer)
main.add_module(pxdClusterizer)
main.add_module(svdClusterizer)
main.add_module(eventCounter)
main.add_module(spCreatorPXD)
main.add_module(spCreatorSVD)
main.add_module(mctrackfinder)

main.add_module(exportXML)

main.add_module(sp2thConnector)
main.add_module(trackCandConverter)
main.add_module(sptcReferee)
main.add_module(filterCalc)
main.add_module(filterCalc2)

# Process events
process(main)

print(statistics)
