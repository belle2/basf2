#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

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

clusterType = 'fullClusterizer'
if useSimpleClusterizer:
    clusterType = 'simpleClusterizer'
particleGenType = 'pGun'
if useEvtGen:
    particleGenType = 'evtGen'
eDepType = 'eDepNo'
if useEDeposit:
    eDepType = 'eDepYes'

print '{events:} events, {numTracks:} tracksEach, Between {minP:} And {maxP:} GeV, Seed {theSeed:}, tMin {minTheta:}, tMax{maxTheta:}, phiMin {minPhi:}, phiMax{maxPhi:} - {genType:}!'.format(
    events=numEvents,
    numTracks=numTracks,
    minP=momentumMin,
    maxP=momentumMax,
    theSeed=seed,
    minTheta=thetaMin,
    maxTheta=thetaMax,
    minPhi=phiMin,
    maxPhi=phiMax,
    genType=particleGenType,
    )

simOutputFileName = \
    '{events:}simulatedEventsAndSeed{seed:}with{evtType:}Using{clusterType:}-{eDep:}May2014phi{phi:}.root'.format(
    events=numEvents,
    seed=seed,
    evtType=particleGenType,
    clusterType=clusterType,
    eDep=eDepType,
    phi=phiMax,
    )

print ''
print ' entering particleSim and simulate using {clusterType:}. {eDep:} writing in file: {ofileName:}'.format(clusterType=clusterType,
        eDep=eDepType, ofileName=simOutputFileName)
print ''

set_log_level(LogLevel.ERROR)
set_random_seed(seed)

#### register die restlichen modules (ab hier kommt nur noch Zeugs, wo Umstellen nur auf eigene Gefahr gilt)
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

# Mix some background to simulation data
bgmixer = register_module('MixBkg')
bgmixer.param('BackgroundFiles', ['rof*.root'])
bgmixer.param('AnalysisMode', False)
bgmixer.set_log_level(LogLevel.INFO)

gearbox = register_module('Gearbox')
# This file contains the VXD (no Telescopes) beam test geometry including the real PCMAG magnetic field
# gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB.xml')

particlegun = register_module('ParticleGun')
param_pGun = {  # 13: muons, 211: charged pions, 521: b+-Meson
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

particlegun2 = register_module('ParticleGun')
param_pGun2 = {  # 11, electrons, 13: muons, 211: charged pions, 521: b+-Meson, 511 b0-Meson, 300553 Y(4S)
                 # fixed, uniform, normal, polyline, uniformPt, normalPt, inversePt, polylinePt or discrete
                 # 'momentumParams': [momentumMin*10., momentumMax*10.],
    'pdgCodes': [11],
    'nTracks': 2,
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
particlegun2.param(param_pGun2)

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

geometry = register_module('Geometry')
geometry.param('Components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
               'PXD', 'SVD'])  # 'MagneticField',

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')
pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')

simpleClusterizer = register_module('VXDSimpleClusterizer')
simpleClusterizer.logging.log_level = LogLevel.DEBUG
simpleClusterizer.logging.debug_level = 1
simpleClusterizer.param('setMeasSigma', 0)
simpleClusterizer.param('onlyPrimaries', False)
if useEDeposit is False:
    simpleClusterizer.param('energyThresholdU', -0.0001)
    simpleClusterizer.param('energyThresholdV', -0.0001)
    simpleClusterizer.param('energyThreshold', -0.0001)

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

output = register_module('RootOutput')
output.param('outputFileName', simOutputFileName)

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

progress = register_module('Progress')

print ''
print ' entering particleGen using {genType:}. writing in file: {fileName:}'.format(genType=particleGenType,
        fileName=simOutputFileName)
print ''

# print(time.time())

# Create paths
main = create_path()
# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)

if useEvtGen:
  ##folgende Module nur für evtGen:
    main.add_module(evtgeninput)
else:
  ## folgende Module nur für pGun:
    main.add_module(particlegun)
    # if addBG:
        # main.add_module(particlegun2)

main.add_module(g4sim)
if addBG:
    main.add_module(bgmixer)

if useSimpleClusterizer:
    print ''
    main.add_module(simpleClusterizer)
else:
    main.add_module(pxdDigitizer)
    main.add_module(svdDigitizer)
    main.add_module(pxdClusterizer)
    main.add_module(svdClusterizer)

main.add_module(eventCounter)
main.add_module(output)

# Process events
print main
process(main)

# print(time.time())
print statistics
