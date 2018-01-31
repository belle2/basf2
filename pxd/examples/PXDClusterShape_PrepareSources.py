#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##############################################################################
#
# This is an example steering file to use PXD shape correction.
# Shape correction is realize in few steps, see exampes:
#   pxd/examples/PXDClasterShape_PrepareSources.py
#   pxd/examples/PXDClasterShape_CalculateCorrections.py
#   pxd/examples/PXDClasterShape_MergeCorrections.py
#   vxd/examples/DQMPXDClusterShape.py
#
# "UseRealData" to use real data without simulations or simulations (default)
# "PixelKind" to run this task:
# 1) (default=-1) standard calibration based on realistic physics or real data
# 2) special calbration for full range of angles, only for simulation!
#    this case is split for every kind of pixel size (4 cases)
#    partical gun is locate close to sensor with giving pixel kind
#    there are storing only data for giving pixel kind
#    PixelKind=0: 55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
#    Correspond to PixelKind in clusters: 1,2.
#    PixelKind=1: 60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
#    Correspond to PixelKind in clusters: 0,3.
#    PixelKind=2: 70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
#    Correspond to PixelKind in clusters: 5,6.
#    PixelKind=3: 85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]
#    Correspond to PixelKind in clusters: 4,7.
#
# Parameters are not independent, there are some priorities:
#   Priorities: PixelKind, UseRealData, rest is independent
#
# There is possibility to call this macro with list of parameters:
#    basf2 PXDClasterShape_PrepareSources.py -- --h  (show parameters)
#    Example for defaults:
#    basf2 pxd/examples/PXDClasterShape_PrepareSources.py -- -
#
##############################################################################
from basf2 import *
from ROOT import Belle2
from tracking import add_tracking_reconstruction
from simulation import add_simulation
from PXDClusterShape_ParticleGunSetting import PXDClusterShape_ParticleGunSetting

import argparse
parser = argparse.ArgumentParser(description="PXD Cluster Shape: create sources for corrections preparing.")
parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--global-tag',
    dest='global_tag',
    action='store',
    default=None,
    type=str,
    help='Global tag to use at central DB in PNNL')

parser.add_argument('--data-output', dest='DataOutput', action='store_const', const=True, default=True,
                    help='Save data to output file, default = False')

# parser.add_argument('--input-file', dest='input_file', action='store', default=None, type=str,
#                    help='Name of imput file')

parser.add_argument('--SplitSourcePreparing', dest='SplitSourcePreparing', action='store', default=0, type=int,
                    help='Set if prepare sources in one step (=0), first data only (=1), use prepared data (=2), default=0')

parser.add_argument('--EdgeClose', dest='EdgeClose', action='store', default=2, type=int,
                    help='Set how many cols/rows from edge are ignored, default=2')
parser.add_argument('--PixelKind', dest='PixelKind', action='store', default=-1, type=int,
                    help='Set kind of data preparing: -1: IP source, 0: pitch 55 um, 1: 60 um, 2: 70 um, 3: 85 um, default=-1.')
parser.add_argument('--UseRealData', dest='UseRealData', action='store_const', const=False, default=False,
                    help='(experts only) Use real data, not simulations, need to set imput file, unpacking, default=False.')

parser.add_argument('--useShapeCorrection', dest='useShapeCorrection', action='store_const', const=True,
                    default=False, help='Use cluster shape correction, default=False')
parser.add_argument('--use-particlegun', dest='UseParticleGun', action='store_const', const=True,
                    default=False, help='Use particlegun (True, default) or BBbar events (False)')

args = parser.parse_args()

# setup_database(args.local_db, args.global_tag)
# reset_database()
# use_database_chain()
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)
# use_local_database(Belle2.FileSystem.findFile("centraldb/dbcache.txt"), "", True, LogLevel.ERROR)

cl = PXDClusterShape_ParticleGunSetting()

if (args.PixelKind != -1):
    args.UseRealData = False

SpecialLayerNo = int(cl.SetLayerNo(args.PixelKind))
SpecialLadderNo = int(cl.SetLadderNo(args.PixelKind))
SpecialSensorNo = int(cl.SetSensorNo(args.PixelKind))

print("      Setting of arguments: ")
print("                 EdgeClose: ", args.EdgeClose)
print("               UseRealData: ", args.UseRealData)
print("                      PixelKind: ", args.PixelKind)
print("                 SpecialLayerNo: ", SpecialLayerNo)
print("                SpecialLadderNo: ", SpecialLadderNo)
print("                SpecialSensorNo: ", SpecialSensorNo)
print("        useShapeCorrection: ", args.useShapeCorrection)
print("            UseParticleGun: ", args.UseParticleGun)
print("      SplitSourcePreparing: ", args.SplitSourcePreparing)

if (args.UseRealData is True):
    # Limit branches use - necessary minimum (removes MC info if input is from simulation, HLT output)
    branches = ['EventMetaData', 'RawFTSWs', 'RawSVDs', 'RawPXDs']
    if not args.unpacking:
        branches = branches + ['PXDDigits', 'SVDDigits']
    rootinput = register_module('RootInput', branchNames=branches)
else:
    particlegun = register_module('ParticleGun')
    particlegun.param(cl.SetParticleGun(args.PixelKind))
    print_params(particlegun)
    simulation = register_module('FullSim')

eventinfosetter = register_module('EventInfoSetter')
progress = register_module('Progress')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
PXDDIGI = register_module('PXDDigitizer')
PXDCLUST = register_module('PXDClusterizer')
# PXDCLUST = register_module('PXDClusterizer', notUseClusterShape=True)
SVDDIGI = register_module('SVDDigitizer')
SVDCLUST = register_module('SVDClusterizer')
# SVDCLUST.param('positionErrorFactor', 0.75)

PXDUnpacker = register_module('PXDUnpacker')
PXDRawHitSorter = register_module('PXDRawHitSorter')
SVDUnpacker = register_module('SVDUnpacker')
SVDDigitSorter = register_module('SVDDigitSorter')

CDCDigitizer = register_module('CDCDigitizer')

ClShCorrectionFileName = "PXD_ClasterShapeCorrections.root"
ClShCorrectionLogFileName = "PXD_ClasterShapeCorrections.log"

ActivatePXDClusterShapeCorrection = register_module('ActivatePXDClusterShapeCorrection')
ActivatePXDClusterShapeCorrection.param('ClShCorrectionFileName', ClShCorrectionFileName)
ActivatePXDClusterShapeCorrection.param('ClShCorrectionLogFileName', ClShCorrectionLogFileName)

"""
secSetup = \
    ['shiftedL3IssueTestVXDStd-moreThan400MeV_PXDSVD',
     'shiftedL3IssueTestVXDStd-100to400MeV_PXDSVD',
     'shiftedL3IssueTestVXDStd-25to100MeV_PXDSVD'
     ]
tuneValue = 0.22

g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)

TrackFinderMCTruthRecoTrack = register_module('TrackFinderMCTruthRecoTracks')
# TrackFinderMCTruthRecoTrack.logging.log_level = LogLevel.INFO

vxdtf = register_module('VXDTF')
# vxdtf.logging.log_level = LogLevel.DEBUG
# vxdtf.logging.debug_level = 1
param_vxdtf = {'sectorSetup': secSetup,
               # 'GFTrackCandidatesColName': 'caTracks',
               # 'tuneCutoffs': tuneValue,
               # 'displayCollector': 2
               # 'calcQIType': 'kalman'
               }

vxdtf.param(param_vxdtf)

SetupGenfitExtrapolation = register_module('SetupGenfitExtrapolation')
"""

output = register_module('RootOutput')
if (args.SplitSourcePreparing != 1):
    output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size

# set_random_seed(1028307)
# set_random_seed(1111122)
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
if (args.PixelKind == -1):
    geometry.param('components', ['MagneticField', 'PXD', 'SVD', 'CDC'])
else:
    geometry.param('components', ['MagneticField', 'PXD'])

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
# eventCounter.param('stepSize', 25)

# show warnings during processing
# set_log_level(LogLevel.WARNING)

outputFileName = 'pxdClShCalSrc_RealData' + str(args.UseRealData)
outputFileName = outputFileName + '_PixelKind' + str(args.PixelKind)
if (args.SplitSourcePreparing == 1):
    outputFileName = outputFileName + '_' + str(44)
    # outputFileName = outputFileName + '_' + str(args.SplitSourcePreparingIncr)
outputFileName = outputFileName + '.root'
output.param('outputFileName', outputFileName)

# PXD shape calibration module
PXDSHCAL = register_module('pxdClusterShapeCalibration')

# Set the region close edge where remove cluster shape corrections, default=2
PXDSHCAL.param('EdgeClose', args.EdgeClose)
PXDSHCAL.param('UseRealData', args.UseRealData)
PXDSHCAL.param('PixelKindCal', args.PixelKind)
PXDSHCAL.param('SpecialLayerNo', SpecialLayerNo)
PXDSHCAL.param('SpecialLadderNo', SpecialLadderNo)
PXDSHCAL.param('SpecialSensorNo', SpecialSensorNo)
# PXDSHCAL.set_log_level(LogLevel.INFO)
# PXDSHCAL.set_log_level(LogLevel.DEBUG)
# PXDSHCAL.set_debug_level(140)

# ============================================================================
# Do the simulation
main = create_path()
if ((args.SplitSourcePreparing != 2) and (args.UseRealData is False)):
    main.add_module(eventinfosetter)
main.add_module(progress)
if (args.UseRealData is True):
    main.add_module(gearbox)
    main.add_module(geometry)
    if args.unpacking:
        PXDUnpacker = register_module('PXDUnpacker')
        PXDRawHitSorter = register_module('PXDRawHitSorter')
        SVDUnpacker = register_module('SVDUnpacker')
        SVDDigitSorter = register_module('SVDDigitSorter')
    main.add_module(rootinput)
else:
    if (args.SplitSourcePreparing != 2):
        if (args.PixelKind == -1):
            if (args.UseParticleGun is True):
                # generate simple particle gun events:
                main.add_module(particlegun)
            else:
                # generate BBbar events:
                main.add_module('EvtGenInput')
            add_simulation(main,
                           components=['MagneticField', 'PXD', 'SVD', 'CDC'],
                           usePXDDataReduction=False)

# if (args.SplitSourcePreparing != 2):
if (args.SplitSourcePreparing == 0):
    if (args.PixelKind == -1):
        if (args.useShapeCorrection is True):
            main.add_module(ActivatePXDClusterShapeCorrection)
        main.add_module(PXDDIGI)   # In case of changes in digitizer
        main.add_module(PXDCLUST)  # Because of wrong clusterization in existing source
        main.add_module(SVDDIGI)   # In case of changes in digitizer
        main.add_module(SVDCLUST)  # In case of changes in clusterizer
        add_tracking_reconstruction(main,
                                    pruneTracks=False,
                                    components=['MagneticField', 'PXD', 'SVD', 'CDC'],
                                    additionalTrackFitHypotheses=[211, -211, 11, -11],
                                    skipGeometryAdding=False,
                                    reco_tracks="RecoTracks"
                                    )
        """ This work for VXD only, so better use add_tracking_reconstruction.
        main.add_module(SetupGenfitExtrapolation)
        main.add_module(vxdtf)
        main.add_module(TrackFinderMCTruthRecoTrack)
        main.add_module('DAFRecoFitter')
        # main.add_module('KalmanRecoFitter')
        """
    else:
        main.add_module(gearbox)
        main.add_module(geometry)
        # generate simple particle gun events:
        main.add_module(particlegun)
        main.add_module(simulation)
        main.add_module(PXDDIGI)
        main.add_module(PXDCLUST)

if (args.SplitSourcePreparing == 2):  # continuing of tracking in existing BB simulation
    rootinput = register_module('RootInput')
    main.add_module(rootinput)
    main.add_module(gearbox)
    main.add_module(geometry)
    if (args.useShapeCorrection is True):
        main.add_module(ActivatePXDClusterShapeCorrection)
    main.add_module(PXDDIGI)   # In case of changes in digitizer
    main.add_module(PXDCLUST)  # Because of wrong clusterization in existing source
    main.add_module(SVDDIGI)   # In case of changes in digitizer
    main.add_module(SVDCLUST)  # In case of changes in clusterizer
    add_tracking_reconstruction(main,
                                pruneTracks=False,
                                components=['MagneticField', 'PXD', 'SVD', 'CDC'],
                                additionalTrackFitHypotheses=[211, -211, 11, -11],
                                skipGeometryAdding=False,
                                reco_tracks="RecoTracks"
                                )
if (args.SplitSourcePreparing != 1):
    main.add_module(PXDSHCAL)

# main.add_module("PrintCollections")
# if ((args.SplitSourcePreparing == 1) or (args.DataOutput is True)):
main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
#
