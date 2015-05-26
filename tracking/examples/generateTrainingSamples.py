#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
from sys import argv
from simulation import add_simulation
import glob

# Parameters
nEvents = 1
useEvtGen = True  # if false ParticleGun is used
useRootInput = False  # read pre-simulated events from root-input
useRootOutputAfterSim = False  # use rootoutput after simulation
useRootBeforeSampleGeneration = False  # use rootoutput just before the invokation of the sample generating module

# In-/Output filenames (dummys for now)
inFileName = 'EvtGen_nobg_25k.root'
outFileName = 'EvtGen_nobg_25k.root'

# ParticleGun parameters
nTracks = 15  # number of tracks
# momentum
momentumMin = .01  # transverse min momentum, GeV/c
momentumMax = 1.4  # transverse max momentum, GeV/c
# angles [degrees]
thetaMin = 15.0
thetaMax = 153.
phiMin = 0.
phiMax = 360.

# SectorMaps and other settings for the VXDTF
# sectorMap setup, for additional PXD: SVD -> PXDSVD (at the end of the names)
secSetup = [
    'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-moreThan500MeV_SVD',
    'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-125to500MeV_SVD',
    'secMapEvtGenAndPGunWithSVDGeo2p2OnR13760Nov2014SVDStd-30to125MeV_SVD']

tuneValue = 0.06  # expands the set cutoffs in the VXDTF in percent

# background file setup
bkgdir = '/home/maldi/belle2FW/bkg/'  # paht to background files
bkgfiles = glob.glob(bkgdir + "*.root")

# module registration
# EventInfoSetter
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [nEvents])

# EventInfoPrinter
eventinfoprinter = register_module('EventInfoPrinter')

# Progress
progress = register_module('Progress')
progress.param('maxN', 2)

# Root In/Output
rootinput = register_module('RootInput')
rootinput.param('inputFileName', inFileName)
rootoutput = register_module('RootOutput')
rootoutput.param('outputFileName', outFileName)

# event generation
evtgen = register_module('EvtGenInput')

particlegun = register_module('ParticleGun')
param_particlegun = {
    'pdgCodes': [13, -13],
    'nTracks': nTracks,
    'momentumGeneration': 'uniformPt',
    'momentumParams': [momentumMin, momentumMax],
    'thetaGeneration': 'uniform',
    'thetaParams': [thetaMin, thetaMax],
    'phiGeneration': 'uniform',
    'phiParams': [phiMin, phiMax],
    'vertexGeneration': 'uniform',
    'xVertexParams': [-.01, .01],
    'yVertexParams': [-.01, .01],
    'zVertexParams': [-0.5, 0.5],
}
particlegun.param(param_particlegun)

# gearbox, geometry and fullsim (for non-default settings)
gearbox = register_module('Gearbox')

geometry = register_module('Geometry')
geometry.param('components', ['BeamPipe', 'MagneticFieldConstant4LimitedRCDC',
                              'PXD', 'SVD'])  # 'components', ['BeamPipe', 'MagneticFieldConstant4LimitedRCDC',........

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)  # need for MCTrackfinder to work correctly

# SpacePoint creation (store in seperate StoreArrays)
spCreatorSVDdouble = register_module('SpacePointCreatorSVD')
spCreatorSVDdouble.param('OnlySingleClusterSpacePoints', False)
spCreatorSVDdouble.param('SpacePoints', 'doubleSP')

spCreatorPXD = register_module('SpacePointCreatorPXD')
spCreatorPXD.param('SpacePoints', 'pxdOnly')

# SpacePoint to TrueHit connector (only setting params here that are needed and differ from default)
sp2thConnector = register_module('SpacePoint2TrueHitConnector')
param_sp2thConnector = {
    'DetectorTypes': ['PXD', 'SVD'],
    'TrueHitNames': ['', ''],
    'ClusterNames': ['', ''],
    'SpacePointNames': ['pxdOnly', 'doubleSP'],
    'storeSeperate': False,
    'registerAll': True,
    'requirePrimary': False,
    'requireProximity': False,
}
sp2thConnector.param(param_sp2thConnector)

# for now misuse the VXDTF to get three hit combinations
vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
vxdtf.logging.debug_level = 1
param_vxdtf = {
    # NOTE: the activateAlwaysTrueNHit is currently not compiled (in the head relase!) into the code
    # setting the filters to false individually
    'sectorSetup': secSetup,
    'GFTrackCandidatesColName': 'vxdTracks',
    'activateAlwaysTrue3Hit': [True],
    'activateAlwaysTrue4Hit': [True],
    'activateDistance3D': [True],
    'activateDistanceXY': [True],
    'activateDistanceZ': [False],
    # 'activateAngles3D': [False],
    # 'activateAngles3DHioC': [False],
    # 'activateAnglesXYHioC': [False],
    # 'activateDeltaDistance2IPHioC': [False],
    # 'activateDeltaPtHioC': [False],
    # 'activateSlopeRZ': [False],
    # 'activateZigZagXY': [False],
    'filterOverlappingTCs': 'none',
    'killEventForHighOccupancyThreshold': 100000,
    'tuneCutoffs': tuneValue,
}
vxdtf.param(param_vxdtf)

# MCTrackFinder
mcTrackFinder = register_module('TrackFinderMCTruth')
mcTrackFinder.logging.log_level = LogLevel.INFO
param_mctrackfinder = {
    'UseCDCHits': 0,
    'UseSVDHits': 1,
    'UsePXDHits': 0,
    'Smearing': 0,
    'UseClusters': True,
    'MinimalNDF': 5,
    'WhichParticles': ['primary'],
    'GFTrackCandidatesColName': 'mcTracks',
    'TrueHitMustExist': True,
}
mcTrackFinder.param(param_mctrackfinder)

# convert trackcands from VXDTF to SPTCs
tcconverter = register_module('GFTC2SPTCConverter')
tcconverter.logging.log_level = LogLevel.INFO
param_tcconverter = {
    'NoSingleClusterSVDSP': 'doubleSP',
    'PXDClusterSP': 'pxdOnly',
    'checkNoSingleSVDSP': True,
    'useSingleClusterSP': False,
    'checkTrueHits': False,
    'skipCluster': False,
    # 'genfitTCName': 'mcTracks',
    'genfitTCName': 'vxdTracks',
    'SpacePointTCName': 'SPTracks',
}
tcconverter.param(param_tcconverter)

# generate training samples
samplesgenerator = register_module('ThreeHitSamplesGenerator')
samplesgenerator.logging.log_level = LogLevel.INFO
samplesgenerator.logging.debug_level = 1000
param_samplesgenerator = {
    'containerName': 'SPTracks',
    'outputFileName': ['threeHitSamplesMConly', 'recreate'],
}
samplesgenerator.param(param_samplesgenerator)

set_log_level(LogLevel.WARNING)
set_random_seed(1234)

# Path
main = create_path()
if not useRootInput:
    main.add_module(eventinfosetter)
    main.add_module(eventinfoprinter)
    # add the generation (EvtGen or PartGun)
    if useEvtGen:
        main.add_module(evtgen)
    else:
        main.add_module(particlegun)

    # main.add_module(gearbox)  # add gearbox and geometry in before add_simulation so that they are in the right order
    # main.add_module(geometry)
    # main.add_module(g4sim)  # add g4sim with custom settings
    # add the simulation (including clusterizing and digitizing)
    # NOTE: using RSVD only here (not RCDC)
    add_simulation(main, ['BeamPipe', 'MagneticFieldConstant4LimitedRSVD', 'PXD', 'SVD'], bkgfiles)
else:
    main.add_module(rootinput)
    main.add_module(eventinfoprinter)
    main.add_module(gearbox)
    main.add_module(geometry)

if useRootOutputAfterSim:
    main.add_module(rootoutput)

main.add_module(spCreatorSVDdouble)
main.add_module(spCreatorPXD)
main.add_module(sp2thConnector)

# NOTE: this step is only needed now!
main.add_module(vxdtf)
# main.add_module(mcTrackFinder)
main.add_module(tcconverter)

if useRootBeforeSampleGeneration and not useRootOutputAfterSim:
    main.add_module(rootoutput)

main.add_module(samplesgenerator)

main.add_module(progress)
process(main)

print statistics
