#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from basf2 import *
from time import time

################# start copy from here
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
numEvents = 25
initialValue = 42

momentum = 6.0  # GeV/c
momentum_spread = 0.05  # %
theta = 90.0  # degrees
theta_spread = 0.005  ## degrees (sigma of gaussian)
phi = 180.0  # degrees
phi_spread = 0.005  # degrees (sigma of gaussian)
gun_x_position = 100.  # cm ... 100cm ... outside magnet + plastic shielding + Al scatterer (air equiv.)
# gun_x_position = 40. # cm ... 40cm ... inside magnet
beamspot_size_y = 0.3  # cm (sigma of gaussian)
beamspot_size_z = 0.3  # cm (sigma of gaussian)
useSimpleClusterizer = False  # <----------- hier umschalten zwischen simple(schnell) und full clusterizer(realistisch)!
useEvtGen = False  # <----------- hier umschalten zwischen evtGen( realistische events) und pGun(einfache events)!
useEDeposit = True  # <----- EnergyDeposit für Hits (zum debuggen) damit False funzt, pxd/data/PXD.xml und svd/data/SVD.xml see neutrons auf true setzen
addBG = False  #  <---- adding Background - funzt noch net
usePXD = True

# flags für die pGun
numTracks = 1

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
    minP=momentum,
    maxP=momentum_spread,
    theSeed=initialValue,
    minTheta=theta,
    maxTheta=theta_spread,
    minPhi=phi,
    maxPhi=phi_spread,
    genType=particleGenType,
    )

simOutputFileName = \
    '{events:}simulatedEventsAndSeed{seed:}with{evtType:}Using{clusterType:}-{eDep:}May2014phi{phi:}.root'.format(
    events=numEvents,
    seed=initialValue,
    evtType=particleGenType,
    clusterType=clusterType,
    eDep=eDepType,
    phi=phi,
    )

print ''
print ' entering particleSim and simulate using {clusterType:}. {eDep:} writing in file: {ofileName:}'.format(clusterType=clusterType,
        eDep=eDepType, ofileName=simOutputFileName)
print ''

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)

#### register die restlichen modules (ab hier kommt nur noch Zeugs, wo Umstellen nur auf eigene Gefahr gilt)
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

# Mix some background to simulation data
# bgmixer = register_module('MixBkg')
##bgmixer.param('BackgroundFiles', ['SVDROF*.root'])
# bgmixer.param('BackgroundFiles', ['SVDROF_RBB_LER_1ms_0x.root', 'SVDROF_RBB_HER_1ms_0x.root'])
# bgmixer.param('AnalysisMode', False)
# bgmixer.set_log_level(LogLevel.INFO)

bgmixer = register_module('BeamBkgMixer')
# bgmixer.param('BackgroundFiles', ['SVDROF*.root'])
bgmixer.param('backgroundFiles', ['Coulomb_LER_100us.root'])
bgmixer.param('backgroundFiles', ['Coulomb_HER_100us.root'])
bgmixer.param('backgroundFiles', ['RBB_HER_100us.root'])
bgmixer.param('backgroundFiles', ['RBB_LER_100us.root'])
bgmixer.param('backgroundFiles', ['Touschek_HER_100us.root'])
bgmixer.param('backgroundFiles', ['Touschek_LER_100us.root'])
bgmixer.param('backgroundFiles', ['twoPhoton_200us.root'])

bgmixer.param('components', ['SVD', 'PXD'])
bgmixer.set_log_level(LogLevel.INFO)

gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName',
              'testbeam/vxd/FullTelescopeVXDTB_v2_NOTAlignedAtAll.xml')

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

evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

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

pxdDigitizer = register_module('PXDDigitizer')
svdDigitizer = register_module('SVDDigitizer')

pxdClusterizer = register_module('PXDClusterizer')
svdClusterizer = register_module('SVDClusterizer')

telDigitizer = register_module('TelDigitizer')
telDigitizer.param('ElectronicNoise', 200)
telDigitizer.param('NoiseSN', 5.0)
telClusterizer = register_module('TelClusterizer')

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
eventCounter.logging.log_level = LogLevel.DEBUG
eventCounter.logging.debug_level = 1
eventCounter.param('stepSize', 1)

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
if addBG:
    main.add_module(bgmixer)
main.add_module(g4sim)

if useSimpleClusterizer:
    print ''
    main.add_module(simpleClusterizer)
else:
    main.add_module(pxdDigitizer)
    main.add_module(pxdClusterizer)
    main.add_module(svdDigitizer)
    main.add_module(svdClusterizer)

main.add_module(eventCounter)
main.add_module(output)

# Process events
print main
process(main)

# print(time.time())
print statistics
