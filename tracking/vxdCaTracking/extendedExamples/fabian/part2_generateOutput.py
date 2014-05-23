#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

generateSecMap = False  # <----------- hier umschalten zwischen secMapGenerierung und VXDTF!

################# start copy from here
useSimpleClusterizer = True  # <----------- hier umschalten zwischen simple(schnell) und full clusterizer(realistisch)!
useEvtGen = False  # <----------- hier umschalten zwischen evtGen( realistische events) und pGun(einfache events)!
useEDeposit = True  # <----- EnergyDeposit für Hits (zum debuggen) damit False funzt, pxd/data/PXD.xml und svd/data/SVD.xml see neutrons auf true setzen
addBG = True  #  <---- adding Background - funzt noch net

numEvents = 20
seed = 1

# flags für die pGun
numTracks = 1
# transverseMomentum:
momentumMin = 0.25  # GeV/c
momentumMax = 0.25  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 35.0  # degrees
thetaMax = 80.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees
################# end Copy here

#### filterCalc & VXDTF:
pTcuts = [0.05, 0.5]
setupFileName = 'belle2SimpleCaseTet35t80phi0t360'
# setupFileName = "b2Theta70to80Phi0to90"
# secSetup = ['belle2SimpleCaseSVD-50to500MeV_SVD'] # hier neuen namen fürs xml-file eintragen
secSetup = ['belle2SimpleCaseTet35t80phi0t360SVD-50to500MeV_SVD']  # hier neuen namen fürs xml-file eintragen
# secSetup = ['sectorList_evtNormSecHIGH_SVD', 'sectorList_evtNormSecMED_SVD', 'sectorList_evtNormSecLOW_SVD'] #standardDings für evtGen
secConfigU = [0., 0.5, 1.0]
secConfigV = [0., 0.33, 0.67, 1.0]
qiType = 'circleFit'
filterOverlaps = 'hopfield'
collector_info = 1
# TFInfoOutputPath = 'datenDeltaSlopeZOverS/'
TFInfoOutputPath = 'datenAngle3D/'
activateCollector = 2  # 0 is aus, 2 is fullAnalyze

param_vxdtf = {  # 'activateZigZagXY': [False, True, True], # WARNING: aktiviere den, wenn du die normalen 3 passes benimmst!
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
    'activateDistanceXY': [False],
    'activateDistance3D': [False],
    'activateDistanceZ': [False],
    'activateSlopeRZ': [False],
    'activateNormedDistance3D': [False],
    'activateAngles3D': [True],
    'activateAnglesXY': [False],
    'activateAnglesRZ': [False],
    'activateDeltaSlopeRZ': [False],
    'activateDistance2IP': [False],
    'activatePT': [False],
    'activateHelixParameterFit': [False],
    'activateDeltaSOverZ': [False],
    'activateDeltaSlopeZOverS': [False],
    'activateDeltaPt': [False],
    'activateDeltaDistance2IP': [False],
    'activateAngles3DHioC': [False],
    'activateAnglesXYHioC': [False],
    'activateAnglesRZHioC': [False],
    'activateDeltaSlopeRZHioC': [False],
    'activateDistance2IPHioC': [False],
    'activatePTHioC': [False],
    'activateHelixParameterFitHioC': [False],
    'activateDeltaPtHioC': [False],
    'activateDeltaDistance2IPHioC': [False],
    'activateZigZagXY': [False],
    'activateZigZagRZ': [False],
    'activateCircleFit': [False],
    'tuneCircleFit': [0.00000001],
    'displayCollector': activateCollector,
    }

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
    seed = int(argv[1])
else:
    seed = int(argv[2])
    numEvents = int(argv[1])

set_log_level(LogLevel.ERROR)
set_random_seed(seed)

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
               'PXD', 'SVD'])

# geometry.set_log_level(LogLevel.INFO) INFO if set to true, complete list of components can be found...

# Show progress of processing
progress = register_module('Progress')

rootFileName = \
    '{events:}simulatedEventsAndSeed{seed:}with{evtType:}Using{clusterType:}-{eDep:}May2014phi{phi:}.root'.format(
    events=numEvents,
    seed=seed,
    evtType=particleGenType,
    clusterType=clusterType,
    eDep=eDepType,
    phi=phiMax,
    )

print ''
print ' entering trackingTests reading file {fileName:}'.format(fileName=rootFileName)
print ''

setupFileNamesvd = setupFileName + 'SVD'
filterCalc2 = register_module('FilterCalculator')
filterCalc2.logging.log_level = LogLevel.DEBUG
filterCalc2.logging.debug_level = 1
param_fCalc2 = {
    'detectorType': ['SVD'],
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
exportXML.param('sampleThreshold', [10, 1000])  # first for small quantiles, second for normal ones
exportXML.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
exportXML.param('smallSampleQuantiles', [0, 1])  # first for min, second for max

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 250)

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1  # 175
vxdtf.param(param_vxdtf)

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')
vxdtf_dqm.param('GFTrackCandidatesColName', 'caTracks')

analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.INFO
analyzer.logging.debug_level = 1
param_analyzer = {
    'printExtentialAnalysisData': False,
    'caTCname': 'caTracks',
    'collectorDisplayId': collector_info,
    'collectorFilePath': TFInfoOutputPath,
    }
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
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQM-VXDTFdemo.root')  # File to save histograms
main.add_module(histo)
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
