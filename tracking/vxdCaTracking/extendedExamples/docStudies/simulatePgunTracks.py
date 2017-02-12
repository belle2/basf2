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
                                 setup_realClusters,
                                 setup_spCreatorSVD,
                                 setup_simpleClusters,
                                 setup_mcTF)


# ################
useSimpleClusterizer = True
useEvtGen = False
usePGun = True
# useEDeposit: If you want to work w/o E-deposit, edit pxd/data/PXD.xml and svd/data/SVD.xml,
# where you have to activate see neutrons = true:
useEDeposit = True
useMultipleScattering = False
# usePXD: useful only for generateSecMap = false, activates secMaps for PXD+SVD and changes settings for the
# TrackFinderMCTruth too.
usePXD = False
useBField = False
doSmearVertex = False
allowParticleDecay = False
simVerbose = True
# ################

# Important parameters:
numEvents = 25000  # can be overridden by the parameters given via terminal
initialValue = 12  # want random events, if set to 0


# flags for the pGun
numTracks = 1
# transverseMomentum:
momentumMin = 0.075  # GeV/c
momentumMax = 0.075  # %
# theta: starting angle of particle direction in r-z-plane
thetaMin = 90.0  # degrees
thetaMax = 90.0  # degrees
# phi: starting angle of particle direction in x-y-plane (r-phi-plane)
phiMin = 0.0  # degrees
phiMax = 360.0  # degrees
# 13: muons, 211: charged pions, 48: geantino (a test particle without interactions)
pdgCODES = [211]


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
pdgCODES2 = [211]


MyLogLevel = LogLevel.INFO
MyDebugLevel = 10

rootIOFileName = ""

# if len(argv) is 1:
# print('no arguments given, using standard values')
# elif len(argv) is 2:
# initialValue = int(argv[1])
# print('1 argument given, new value for seed: ' + str(initialValue))
# elif len(argv) is 3:
# initialValue = int(argv[1])
# numEvents = int(argv[2])
# print('2 arguments given, new value for seed: ' + str(initialValue) +
# ' and for numEvents: ' + str(numEvents))

if len(argv) is 1:
    print('no arguments given, using standard values')
if len(argv) > 1:
    momentumMin = float(argv[1])
    momentumMax = float(argv[1])
    print('1st argument given, new value for momentumMin: ' + str(momentumMin))
if len(argv) > 2:
    if int(argv[2]) == 1:
        useEDeposit = True
    else:
        useEDeposit = False
    print('2nd argument given, new value for useEDeposit: ' + str(useEDeposit))
if len(argv) > 3:
    if int(argv[3]) == 1:
        useMultipleScattering = True
    else:
        useMultipleScattering = False
    print('3rd arguments given, new value for useMultipleScattering: ' + str(useMultipleScattering))
if len(argv) > 4:
    pdgCODES = [int(argv[4]), - int(argv[4])]
    print('4th arguments given, new values for pdgCODES: ' + str(pdgCODES[0]) + '/' + str(pdgCODES[1]))

set_log_level(LogLevel.WARNING)
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


if (numTracks != 0 and usePGun is True):
    particlegun = setup_pGun(
        pdgCODES=pdgCODES, numTracks=numTracks, momParams=[
            momentumMin, momentumMax], thetaParams=[
            thetaMin, thetaMax], phiParams=[
                phiMin, phiMax], logLevel=LogLevel.WARNING,
        smearVertex=doSmearVertex)
    rootIOFileName += 'pGun1_' + str(numTracks) + 'T'

if (numTracks2 != 0 and usePGun is True):
    particlegun2 = setup_pGun(
        pdgCODES=pdgCODES2, numTracks=numTracks2, momParams=[
            momentumMin2, momentumMax2], thetaParams=[
            thetaMin2, thetaMax2], phiParams=[
                phiMin2, phiMax2], logLevel=LogLevel.WARNING,
        smearVertex=doSmearVertex)
    rootIOFileName += 'pGun2_' + str(numTracks2) + 'T'

runLogFName = rootIOFileName + ".log"


evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING


geometry = register_module('Geometry')
if useBField:
    geometry.param('components', ['MagneticFieldConstant4LimitedRSVD', 'SVD'])
else:
    geometry.param('components', ['SVD'])


rootOutputM = register_module('RootOutput')
rootIOFileName += '.root'
rootOutputM.param('outputFileName', rootIOFileName)


progress = register_module('Progress')

print('')
print('')


log_to_file('createSim' + str(initialValue) + '_' + str(numEvents) + '.log', append=False)
# Create paths
main = create_path()

# beam parameters
# beamparameters = add_beamparameters(main, "Y4S")
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

if useEDeposit:
    seeNeutrons = "false"
else:
    seeNeutrons = "true"
main.add_module("Gearbox", override=[("/DetectorComponent[@name='SVD']//SeeNeutrons", seeNeutrons, "")])
main.add_module(geometry)

setup_sim(
    path=main,
    useEDeposit=useEDeposit,
    useMultipleScattering=useMultipleScattering,
    allowDecay=allowParticleDecay,
    verbose=simVerbose)

setup_simpleClusters(main, onlyPrimaries=True, useEDeposit=False)

# main.add_module(simpleClusterizer)

setup_spCreatorSVD(main, nameOutput='spacePoints')

main.add_module(register_module("StudyMaterialEffects"))

# main.add_module(rootOutputM)


# for m in main.modules():
# print_params(m)

print(main)


set_nprocesses(6)

process(main)

print(statistics)
