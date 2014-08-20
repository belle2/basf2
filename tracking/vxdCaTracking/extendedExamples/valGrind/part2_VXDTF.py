#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

generateSecMap = False  # <----------- hier umschalten zwischen secMapGenerierung und VXDTF!

################# start copy from here
useSimpleClusterizer = False  # <----------- hier umschalten zwischen simple(schnell) und full clusterizer(realistisch)!
useEvtGen = True  # <----------- hier umschalten zwischen evtGen( realistische events) und pGun(einfache events)!
useEDeposit = True  # <----- EnergyDeposit für Hits (zum debuggen) damit False funzt, pxd/data/PXD.xml und svd/data/SVD.xml see neutrons auf true setzen
addBG = False  #  <---- adding Background - funzt noch net
usePXD = False

numEvents = 5
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

useTFDebug = True
TFDebugLevel = 2
killThreshold = 2000
hiocThreshold = 500
cutoffTune = 0.06
minState = 2
minLayer = 4
if usePXD:
    killThreshold = 500000
    hiocThreshold = 500
    minState = 4
    cutoffTune = 0.15
secSetup = ['secMapEvtGenOnR10933June2014SVDStd-moreThan500MeV_SVD',
            'secMapEvtGenOnR10933June2014SVDStd-125to500MeV_SVD',
            'secMapEvtGenOnR10933June2014SVDStd-30to125MeV_SVD']
if usePXD:
    secSetup = ['secMapEvtGenOnR10933June2014VXDStd-moreThan500MeV_PXDSVD',
                'secMapEvtGenOnR10933June2014VXDStd-125to500MeV_PXDSVD',
                'secMapEvtGenOnR10933June2014VXDStd-30to125MeV_PXDSVD']

qiType = 'circleFit'
filterOverlaps = 'hopfield'  # hopfield
collector_info = 0
TFInfoOutputPath = 'datenAngle3D/'
activateCollector = 0  # 0 is aus, 2 is fullAnalyze

param_vxdtf = {  # extended output for filters
                 # ### 3
                 # 'reserveHitsThreshold' : [0.7, 0.5, 0.3],
                 # 'activateAnglesXY': [True],  #### noMagnet
                 # ### withMagnet
                 # 'activateAnglesRZHioC': [True], #### noMagnet
                 # ### withMagnet r51x
                 # True
    'activateBaselineTF': 0,
    'tccMinState': [minState],
    'tccMinLayer': [minLayer],
    'standardPdgCode': 211,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'debugMode': 0,
    'killEventForHighOccupancyThreshold': killThreshold,
    'highOccupancyThreshold': hiocThreshold,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'caTracks',
    'tuneCutoffs': cutoffTune,
    'activateDistanceXY': [True],
    'activateDistance3D': [True],
    'activateDistanceZ': [False],
    'activateSlopeRZ': [True],
    'activateNormedDistance3D': [False],
    'activateAngles3D': [True],
    'activateAnglesXY': [False],
    'activateAnglesRZ': [False],
    'activateDeltaSlopeRZ': [True],
    'activateDistance2IP': [False, False, False],
    'activatePT': [False, False, False],
    'activateHelixParameterFit': [False],
    'activateDeltaSlopeZOverS': [False],
    'activateDeltaSOverZ': [False],
    'activateAngles3DHioC': [True],
    'activateAnglesXYHioC': [True],
    'activateAnglesRZHioC': [False],
    'activateDeltaSlopeRZHioC': [False],
    'activateDistance2IPHioC': [False],
    'activatePTHioC': [False],
    'activateHelixParameterFitHioC': [False],
    'activateDeltaPtHioC': [False],
    'activateDeltaDistance2IPHioC': [False],
    'activateZigZagXY': [False, True, True],
    'activateZigZagRZ': [False],
    'activateDeltaPt': [False, False, False],
    'activateDeltaDistance2IP': [False],
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

vxdtf = register_module('VXDTF')
if useTFDebug:
    vxdtf.logging.log_level = LogLevel.DEBUG
    vxdtf.logging.debug_level = TFDebugLevel  # 175
else:
    vxdtf.logging.log_level = LogLevel.INFO
vxdtf.param(param_vxdtf)

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

rootinput = register_module('RootInput')
rootinput.param('inputFileName', rootFileName)

# Create paths
main = create_path()
# Add modules to paths
main.add_module(rootinput)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(vxdtf)
main.add_module(analyzer)

# Process events
process(main)

print statistics
