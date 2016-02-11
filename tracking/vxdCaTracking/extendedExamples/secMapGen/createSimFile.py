#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from subprocess import call
from sys import argv
from beamparameters import add_beamparameters

# This is tracking/vxdCaTracking/extendedExamples/scripts/setup_modules.py
from VXDTF.setup_modules import (setup_sim,
                                 setup_pGun,
                                 setup_gfTCtoSPTCConverters,
                                 setup_realClusters,
                                 setup_spCreatorPXD,
                                 setup_spCreatorSVD,
                                 setup_mcTF)


# ################
useSimpleClusterizer = False
useEvtGen = False
usePGun = True
# useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
# where you have to activate see neutrons = true:
useEDeposit = True
# usePXD: useful only for generateSecMap = false, activates secMaps for PXD+SVD and changes settings for the
# TrackFinderMCTruth too.
usePXD = False
# ################


# Important parameters:
numEvents = 2  # can be overridden by the parameters given via terminal
initialValue = 57  # want random events, if set to 0


# flags for the pGun
numTracks = 10
# transverseMomentum:
momentumMin = 0.1  # GeV/c
momentumMax = 0.145  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 65.0  # degrees
thetaMax = 85.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 315.  # degrees
phiMax = 360.  # degrees
# 13: muons, 211: charged pions
pdgCODES = [13]


# flags for the 2nd pGun
numTracks2 = 0
# transverseMomentum:
momentumMin2 = 0.3  # GeV/c
momentumMax2 = 3.5  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin2 = 65.0  # degrees
thetaMax2 = 85.  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin2 = 315.  # degrees
phiMax2 = 360.  # degrees
# 13: muons, 211: charged pions
pdgCODES2 = [13]


MyLogLevel = LogLevel.INFO
MyDebugLevel = 10

rootIOFileName = ""

if len(argv) is 1:
    print('no arguments given, using standard values')
elif len(argv) is 2:
    initialValue = int(argv[1])
    print('1 argument given, new value for seed: ' + str(initialValue))
elif len(argv) is 3:
    initialValue = int(argv[1])
    numEvents = int(argv[2])
    print('2 arguments given, new value for seed: ' + str(initialValue) +
          ' and for numEvents: ' + str(numEvents))

set_log_level(LogLevel.ERROR)
set_random_seed(initialValue)


if useSimpleClusterizer:
    rootIOFileName += 'simpleClusterizer'
if useEvtGen:
    rootIOFileName += 'evtGen'
eDepType = 'eDepNo'
if useEDeposit is False:
    rootIOFileName += ' NOeDep'

rootIOFileName += 'seed' + str(initialValue) + 'nEv' + str(numEvents)


eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [numEvents])


eventinfoprinter = register_module('EventInfoPrinter')


gearbox = register_module('Gearbox')


setupGenfit = register_module('SetupGenfitExtrapolation')

if (numTracks != 0 and usePGun is True):
    particlegun = setup_pGun(
        pdgCODES=pdgCODES, numTracks=numTracks, momParams=[
            momentumMin, momentumMax], thetaParams=[
            thetaMin, thetaMax], phiParams=[
                phiMin, phiMax], logLevel=LogLevel.WARNING)
    rootIOFileName += 'pGun1_' + str(numTracks) + 'T'


if (numTracks2 != 0 and usePGun is True):
    particlegun2 = setup_pGun(
        pdgCODES=pdgCODES2, numTracks=numTracks2, momParams=[
            momentumMin2, momentumMax2], thetaParams=[
            thetaMin2, thetaMax2], phiParams=[
                phiMin2, phiMax2], logLevel=LogLevel.WARNING)
    rootIOFileName += 'pGun2_' + str(numTracks2) + 'T'


evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING


geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD',
                              'PXD', 'SVD'])


rootOutputM = register_module('RootOutput')
rootIOFileName += '.root'
rootOutputM.param('outputFileName', rootIOFileName)


progress = register_module('Progress')

print('')
print('')


if useSimpleClusterizer:
    simpleClusterizer = register_module('VXDSimpleClusterizer')
    simpleClusterizer.logging.log_level = LogLevel.INFO
    simpleClusterizer.logging.debug_level = 1
    simpleClusterizer.param('setMeasSigma', 0)
    simpleClusterizer.param('onlyPrimaries', True)
    if useEDeposit is False:
        simpleClusterizer.param('energyThresholdU', -0.0001)
        simpleClusterizer.param('energyThresholdV', -0.0001)
        simpleClusterizer.param('energyThreshold', -0.0001)


eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 1)


g4sim = setup_sim()


log_to_file('createSim' + str(initialValue) + '_' + str(numEvents) + '.log', append=False)
# Create paths
main = create_path()

# beam parameters
beamparameters = add_beamparameters(main, "Y4S")
# beamparameters = add_beamparameters(main, "Y1S")
# beamparameters.param("generateCMS", True)
# beamparameters.param("smearVertex", False)
# beamparameters.param("smearEnergy", False)
# print_params(beamparameters)

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(progress)
if useEvtGen:
    # # following modules only for evtGen:
    if usePGun:
        if (numTracks != 0):
            main.add_module(particlegun)
        if (numTracks2 != 0):
            main.add_module(particlegun2)
    main.add_module(evtgeninput)
else:
    # # following modules only for pGun:
    if (numTracks != 0):
        main.add_module(particlegun)
    if (numTracks2 != 0):
        main.add_module(particlegun2)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(setupGenfit)
main.add_module(g4sim)
if useSimpleClusterizer:
    main.add_module(simpleClusterizer)
else:
    setup_realClusters(main, usePXD=True)  # needed since 2gftc-converter does not work without it
setup_spCreatorSVD(path=main, nameOutput='nosingleSP', createSingleClusterSPs=False, logLevel=LogLevel.INFO)
setup_spCreatorPXD(path=main, nameOutput='pxdOnly', logLevel=LogLevel.INFO)  # needed since 2gftc-converter does not work without it
setup_mcTF(path=main, nameOutput='mcTracks', usePXD=usePXD, logLevel=LogLevel.INFO)

setup_gfTCtoSPTCConverters(
    path=main,
    pxdSPs='pxdOnly',
    svdSPs='nosingleSP',
    gfTCinput='mcTracks',
    sptcOutput='checkedSPTCs',
    usePXD=False,
    logLevel=LogLevel.WARNING)

main.add_module(rootOutputM)


process(main)

print(statistics)
