#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

#################

generateSecMap = True  # <----------- hier umschalten zwischen secMapGenerierung und VXDTF!
useSimpleClusterizer = True
useEvtGen = True
useEDeposit = True  # <----- damit False funzt, pxd/data/PXD.xml und svd/data/SVD.xml see neutrons auf true setzen

#################
# Important parameters:

numTracks = 10
numEvents = 2000
initialValue = 1  # want random events, if set to 0
phiMax = 90.  # degrees

# parameters for the secMap-calculation:
pTcuts = [0.05, 0.5]
# setupFileName = "belle2SimpleCaseTet70t80phi0t90"
setupFileName = 'belle2SimpleCaseTestingTestbeam0'
secConfigU = [0., 0.5, 1.0]
secConfigV = [0., 0.33, 0.67, 1.0]

# secSetup = ['belle2SimpleCaseSVD-50to500MeV_SVD'] # hier neuen namen fürs xml-file eintragen
secSetup = ['belle2SimpleCaseTet70t80phi0t90SVD-50to500MeV_SVD']  # hier neuen namen fürs xml-file eintragen
# data/tracking/belle2SimpleCaseTet70t80phi0t90SVD-50to500MeV_SVD.xml
qiType = 'circleFit'
filterOverlaps = 'hopfield'

clusterType = 'fullClusterizer'
if useSimpleClusterizer:
    clusterType = 'simpleClusterizer'
particleGenType = 'pGun'
if useEvtGen:
    particleGenType = 'evtGen'
eDepType = 'eDepNo'
if useEDeposit:
    eDepType = 'eDepYes'

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

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
               'PXD', 'SVD'])

# geometry.set_log_level(LogLevel.INFO) INFO if set to true, complete list of components can be found...

# Show progress of processing
progress = register_module('Progress')

rootFileName = \
    '{events:}simulatedEventsAndSeed{seed:}Using{clusterType:}-{eDep:}April2014phi{phi:}.root'.format(events=numEvents,
        seed=initialValue, clusterType=clusterType, eDep=eDepType, phi=phiMax)

print ''
print ' entering trackingTests reading file {fileName:}'.format(fileName=rootFileName)
print ''

setupFileNamesvd = setupFileName + 'SVD'
filterCalc2 = register_module('FilterCalculator')
filterCalc2.logging.log_level = LogLevel.DEBUG
filterCalc2.logging.debug_level = 30
param_fCalc2 = {  # -1 = VXD, 0 = PXD, 1 = SVD
                  # # completely different to values of Belle2-detector
                  # arbitrary origin
                  # 1
    'detectorType': 1,
    'maxXYvertexDistance': 200.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 1,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigU,
    'sectorConfigV': secConfigV,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 1,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamesvd,
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
    }
filterCalc2.param(param_fCalc2)

# using one export module only
exportXML = register_module('ExportSectorMap')
exportXML.logging.log_level = LogLevel.DEBUG
exportXML.logging.debug_level = 30
exportXML.param('rootFileName', setupFileName)
exportXML.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
exportXML.param('sampleThreshold', [1, 10])  # first for small quantiles, second for normal ones
exportXML.param('sampleQuantiles', [0, 1])  # first for min, second for max
exportXML.param('smallSampleQuantiles', [0, 1])  # first for min, second for max

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 250)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 2  # 175
# calcQIType:
# Supports 'kalman', 'circleFit' or 'trackLength.
# 'circleFit' has best performance at the moment

# filterOverlappingTCs:
# Supports 'hopfield', 'greedy' or 'none'.
# 'hopfield' has best performance at the moment
param_vxdtf = {  # extended output for filters
    'activateBaselineTF': 0,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'standardPdgCode': -13,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'debugMode': 0,
    'killEventForHighOccupancyThreshold': 55555,
    'highOccupancyThreshold': 11111,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'caTracks',
    'tuneCutoffs': 0.0001,
    'activateDistanceXY': [True],
    'activateDistance3D': [True],
    'activateAngles3DHioC': [True],
    'activateAnglesXYHioC': [False],
    'activateDeltaSlopeRZHioC': [True],
    'activateDistance2IPHioC': [False],
    'activatePTHioC': [True],
    'activateHelixFitHioC': [False],
    'activateDeltaPtHioC': [False],
    'activateDeltaDistance2IPHioC': [False],
    'activateAngles3D': [True],
    'activateAnglesXY': [True],
    'activateAnglesRZ': [False],
    'activateDeltaSlopeRZ': [True],
    'activateDistance2IP': [False],
    'activatePT': [True],
    'activateHelixFit': [False],
    'activateZigZagXY': [True],
    'activateDeltaPt': [True],
    'activateCircleFit': [False],
    'tuneCircleFit': [0.00000001],
    }
vxdtf.param(param_vxdtf)

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 1
param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracks'}
analyzer.param(param_analyzer)

mctrackfinder = register_module('TrackFinderMCTruth')
mctrackfinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 6,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    }
mctrackfinder.param(param_mctrackfinder)

rootinput = register_module('RootInput')
rootinput.param('inputFileName', rootFileName)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(rootinput)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(eventCounter)

if generateSecMap:
  ##folgende Module nur für secMapGen:
    main.add_module(filterCalc2)
    main.add_module(exportXML)
else:
  ## folgende Module nur für secMapTest:
    main.add_module(vxdtf)
    main.add_module(mctrackfinder)
    main.add_module(analyzer)

# Process events
process(main)

print statistics
