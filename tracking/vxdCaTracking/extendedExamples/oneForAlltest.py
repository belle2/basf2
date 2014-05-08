#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv

#################

generateSecMap = True  # <----------- hier umschalten zwischen secMapGenerierung und VXDTF!
useSimpleClusterizer = False
useEvtGen = True
useEDeposit = True  # <----- damit False funzt, pxd/data/PXD.xml und svd/data/SVD.xml see neutrons auf true setzen
doExportXML = False

#################
# Important parameters:

numTracks = 10
numEvents = 250
initialValue = 1  # want random events, if set to 0

# flags für die pGun
numTracks = 10
# transverseMomentum:
momentumMin = 0.25  # GeV/c
momentumMax = 0.25  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 35.0  # degrees
thetaMax = 80.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.  # degrees
phiMax = 360.  # degrees

# parameters for the secMap-calculation:
pTcuts = [0.03, 0.125, 0.5]
# setupFileName = "belle2SimpleCaseTet70t80phi0t90"
setupFileName = 'secMapEvtGenOnR10454May2014'

secConfigUStd = [0., 0.33, 0.67, 1.]
secConfigVStd = [0., 0.33, 0.67, 1.]

secConfigUMini = [0., 0.5, 1.]
secConfigVMini = [0., 0.5, 1.]

secConfigUFine = [
    0.,
    0.2,
    0.4,
    0.6,
    0.8,
    1.,
    ]
secConfigVFine = [
    0.,
    0.2,
    0.4,
    0.6,
    0.8,
    1.,
    ]

# secSetup = ['belle2SimpleCaseSVD-50to500MeV_SVD'] # hier neuen namen fürs xml-file eintragen
secSetup = ['belle2SimpleCaseTet70t80phi0t90SVD-50to500MeV_SVD']  # hier neuen namen fürs xml-file eintragen
# data/tracking/belle2SimpleCaseTet70t80phi0t90SVD-50to500MeV_SVD.xml
qiType = 'circleFit'
filterOverlaps = 'hopfield'

if len(argv) is 1:
    print 'no arguments given, using standard values'
elif len(argv) is 2:
    initialValue = int(argv[1])
    print '1 argument given, new value for seed: ' + str(initialValue)
elif len(argv) is 3:
    initialValue = int(argv[1])
    numEvents = int(argv[2])
    print '2 arguments given, new value for seed: ' + str(initialValue) \
        + ' and for numEvents: ' + str(numEvents)
elif len(argv) is 4:
    initialValue = int(argv[1])
    numEvents = int(argv[2])
    eventAdd = int(argv[3]) * numEvents
    print '3 arguments given, new value for seed: ' + str(initialValue) \
        + ', for numEvents: ' + str(numEvents) + ', for eventAdd: ' \
        + str(eventAdd)
elif len(argv) is 5:
    initialValue = int(argv[1])
    numEvents = int(argv[2])
    eventAdd = int(argv[3]) * numEvents
    tempX = int(argv[4])
    if tempX is 0:
        doExportXML = False
    else:
        doExportXML = True
    print '4 arguments given, new value for seed: ' + str(initialValue) \
        + ', for numEvents: ' + str(numEvents) + ', for eventAdd: ' \
        + str(eventAdd) + ', for doExportXML: ' + str(doExportXML)

clusterType = 'fullClusterizer'
if useSimpleClusterizer:
    clusterType = 'simpleClusterizer'
particleGenType = 'pGun'
if useEvtGen:
    particleGenType = 'evtGen'
eDepType = 'eDepNo'
if useEDeposit:
    eDepType = 'eDepYes'

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

# -1 = VXD, 0 = PXD, 1 = SVD
setupFileNamesvd = setupFileName + 'SVD'
setupFileNamevxd = setupFileName + 'VXD'
setupFileNamesvdMini = setupFileNamesvd + 'Mini'
setupFileNamesvdStd = setupFileNamesvd + 'Std'
setupFileNamesvdFine = setupFileNamesvd + 'Fine'
setupFileNamevxdMini = setupFileNamevxd + 'Mini'
setupFileNamevxdStd = setupFileNamevxd + 'Std'
setupFileNamevxdFine = setupFileNamevxd + 'Fine'

param_filterCalcSVDMini = {
    'detectorType': 1,
    'maxXYvertexDistance': 1.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 1,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUMini,
    'sectorConfigV': secConfigVMini,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamesvdMini,
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
    }
param_filterCalcSVDStd = {
    'detectorType': 1,
    'maxXYvertexDistance': 1.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 1,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUStd,
    'sectorConfigV': secConfigVStd,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamesvdStd,
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
    }
param_filterCalcSVDFine = {
    'detectorType': 1,
    'maxXYvertexDistance': 1.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 1,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUFine,
    'sectorConfigV': secConfigVFine,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamesvdFine,
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
    }

param_filterCalcVXDMini = {
    'detectorType': -1,
    'maxXYvertexDistance': 1.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 1,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUMini,
    'sectorConfigV': secConfigVMini,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamevxdMini,
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
    }
param_filterCalcVXDStd = {
    'detectorType': -1,
    'maxXYvertexDistance': 1.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 1,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUStd,
    'sectorConfigV': secConfigVStd,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamevxdStd,
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
    }
param_filterCalcVXDFine = {
    'detectorType': -1,
    'maxXYvertexDistance': 1.,
    'tracksPerEvent': numTracks,
    'useEvtgen': 1,
    'pTcuts': pTcuts,
    'highestAllowedLayer': 6,
    'sectorConfigU': secConfigUFine,
    'sectorConfigV': secConfigVFine,
    'setOrigin': [0., 0., 0.],
    'magneticFieldStrength': 1.5,
    'testBeam': 0,
    'secMapWriteToRoot': 1,
    'secMapWriteToAscii': 0,
    'rootFileName': [setupFileName, 'UPDATE'],
    'sectorSetupFileName': setupFileNamevxdFine,
    'smearHits': 0,
    'uniSigma': 0.3,
    'noCurler': 1,
    'useOldSecCalc': 0,
    }

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')

particlegun = register_module('ParticleGun')
param_pGun = {  # 13: muons, 211: charged pions
                # fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
    'pdgCodes': [13],
    'nTracks': numTracks,
    'momentumGeneration': 'uniformPt',
    'momentumParams': [momentumMin, momentumMax],
    'thetaGeneration': 'uniform',
    'thetaParams': [thetaMin, thetaMax],
    'phiGeneration': 'uniform',
    'phiParams': [phiMin, phiMax],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0., 0.],
    'yVertexParams': [0., 0.],
    'zVertexParams': [0., 0.],
    }
particlegun.param(param_pGun)

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
               'PXD', 'SVD'])

# geometry.set_log_level(LogLevel.INFO) INFO if set to true, complete list of components can be found...

# Show progress of processing
progress = register_module('Progress')

print ''
print ''

filterCalcSVDMini = register_module('FilterCalculator')
filterCalcSVDMini.logging.log_level = LogLevel.INFO
filterCalcSVDMini.logging.debug_level = 30
filterCalcSVDMini.param(param_filterCalcSVDMini)

filterCalcSVDStd = register_module('FilterCalculator')
filterCalcSVDStd.param(param_filterCalcSVDStd)

# filterCalcSVDFine = register_module('FilterCalculator')
# filterCalcSVDFine.param(param_filterCalcSVDFine)

filterCalcVXDMini = register_module('FilterCalculator')
filterCalcVXDMini.param(param_filterCalcVXDMini)

filterCalcVXDStd = register_module('FilterCalculator')
filterCalcVXDStd.param(param_filterCalcVXDStd)

# filterCalcVXDFine = register_module('FilterCalculator')
# filterCalcVXDFine.param(param_filterCalcVXDFine)

# using one export module only
exportXML = register_module('ExportSectorMap')
exportXML.logging.log_level = LogLevel.DEBUG
exportXML.logging.debug_level = 1
exportXML.param('rootFileName', setupFileName)
exportXML.param('stretchFactor', [0, 0])  # first for small quantiles, second for normal ones
exportXML.param('sampleThreshold', [2, 1000])  # first for small quantiles, second for normal ones
exportXML.param('sampleQuantiles', [0.001, 0.999])  # first for min, second for max
exportXML.param('smallSampleQuantiles', [0, 1])  # first for min, second for max

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')

simpleClusterizer = register_module('VXDSimpleClusterizer')
simpleClusterizer.logging.log_level = LogLevel.DEBUG
simpleClusterizer.logging.debug_level = 1
simpleClusterizer.param('setMeasSigma', 0)
simpleClusterizer.param('onlyPrimaries', True)
if useEDeposit is False:
    simpleClusterizer.param('energyThresholdU', -0.0001)
    simpleClusterizer.param('energyThresholdV', -0.0001)
    simpleClusterizer.param('energyThreshold', -0.0001)

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

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)
if useEDeposit is False:
    g4sim.param('UICommands', [
        '/process/list',
        '/process/inactivate                msc',
        '/process/inactivate              hIoni',
        '/process/inactivate            ionIoni',
        '/process/inactivate              eIoni',
        '/process/inactivate              eBrem',
        '/process/inactivate            annihil',
        '/process/inactivate               phot',
        '/process/inactivate              compt',
        '/process/inactivate               conv',
        '/process/inactivate             hBrems',
        '/process/inactivate          hPairProd',
        '/process/inactivate              muMsc',
        '/process/inactivate             muIoni',
        '/process/inactivate            muBrems',
        '/process/inactivate         muPairProd',
        '/process/inactivate        CoulombScat',
        '/process/inactivate    PhotonInelastic',
        '/process/inactivate     ElectroNuclear',
        '/process/inactivate    PositronNuclear',
        '/process/inactivate              Decay',
        '/process/inactivate         hadElastic',
        '/process/inactivate   NeutronInelastic',
        '/process/inactivate           nCapture',
        '/process/inactivate           nFission',
        '/process/inactivate    ProtonInelastic',
        '/process/inactivate  PionPlusInelastic',
        '/process/inactivate PionMinusInelastic',
        '/process/inactivate  KaonPlusInelastic',
        '/process/inactivate KaonMinusInelastic',
        '/process/inactivate KaonZeroLInelastic',
        '/process/inactivate KaonZeroSInelastic',
        '/process/inactivate AntiProtonInelastic',
        '/process/inactivate AntiNeutronInelastic',
        '/process/inactivate    LambdaInelastic',
        '/process/inactivate AntiLambdaInelastic',
        '/process/inactivate SigmaMinusInelastic',
        '/process/inactivate AntiSigmaMinusInelastic',
        '/process/inactivate SigmaPlusInelastic',
        '/process/inactivate AntiSigmaPlusInelastic',
        '/process/inactivate   XiMinusInelastic',
        '/process/inactivate AntiXiMinusInelastic',
        '/process/inactivate    XiZeroInelastic',
        '/process/inactivate AntiXiZeroInelastic',
        '/process/inactivate OmegaMinusInelastic',
        '/process/inactivate AntiOmegaMinusInelastic',
        '/process/inactivate CHIPSNuclearCaptureAtRest',
        '/process/inactivate muMinusCaptureAtRest',
        '/process/inactivate  DeuteronInelastic',
        '/process/inactivate    TritonInelastic',
        '/process/inactivate      ExtEnergyLoss',
        '/process/inactivate       OpAbsorption',
        '/process/inactivate         OpRayleigh',
        '/process/inactivate            OpMieHG',
        '/process/inactivate         OpBoundary',
        '/process/inactivate              OpWLS',
        '/process/inactivate           Cerenkov',
        '/process/inactivate      Scintillation',
        ])
# "/process/inactivate        StepLimiter"

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(eventCounter)

if useEvtGen:
  ##folgende Module nur für evtGen:
    main.add_module(evtgeninput)
else:
  ## folgende Module nur für pGun:
    main.add_module(particlegun)

main.add_module(g4sim)

if generateSecMap:
  ##folgende Module nur für secMapGen:
    main.add_module(filterCalcSVDMini)
    main.add_module(filterCalcSVDStd)
    # main.add_module(filterCalcSVDFine)
    main.add_module(filterCalcVXDMini)
    main.add_module(filterCalcVXDStd)
    # main.add_module(filterCalcVXDFine)
    if doExportXML:
        main.add_module(exportXML)
else:
  ## folgende Module nur für secMapTest:
    if useSimpleClusterizer:
        print ''
        main.add_module(simpleClusterizer)
    else:
        main.add_module(pxdDigitizer)
        main.add_module(svdDigitizer)
        main.add_module(pxdClusterizer)
        main.add_module(svdClusterizer)
    main.add_module(vxdtf)
    main.add_module(mctrackfinder)
    main.add_module(analyzer)

# Process events
process(main)

print statistics
