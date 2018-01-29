#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file to see DQM of cluster shape of PXD
#
# Sample of external file, particlegun or BBbar events.
#                    (default is particlegun, quicker)
#
# Usage: basf2 DQMPXDClusterShape.py
#      ( basf2 DQMPXDClusterShape.py -- --h )
#        basf2 vxd/examples/DQMPXDClusterShape.py -n 200 --
#                        --filename-histos outDQMHistClShTest.root
#                        --useShapeCorrection
#                        --global-tag "development"
#
# Input: external file with data or None (simulation)
# Output: histograms: DQMPXDClusterShape_Histograms.root or custom...
#
# Author: The Belle II Collaboration                                     *
# Contributors: Peter Kodys                                              *
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
from ROOT import Belle2
from simulation import add_simulation
from pxd import add_pxd_reconstruction
from svd import add_svd_reconstruction
from tracking import add_tracking_reconstruction

import argparse
parser = argparse.ArgumentParser(description="PXD cluster shape DQM for Belle II")
parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--global-tag',
    dest='global_tag',
    action='store',
    default='development',
    type=str,
    help='Global tag to use at central DB in PNNL')

parser.add_argument('--data-output', dest='DataOutput', action='store_const', const=True, default=False,
                    help='Save data to output file, default = False')

parser.add_argument('--useShapeCorrection', dest='useShapeCorrection', action='store_const', const=True,
                    default=False, help='Use cluster shape correction, default=False')

parser.add_argument('--use-real-data', dest='UseRealdata', action='store_const', const=False,
                    default=False, help='Use real data, need set imput file, unpacking')
parser.add_argument('--use-particlegun', dest='UseParticleGun', action='store_const', const=False,
                    default=True, help='Use particlegun (True, default) or BBbar events (False)')
parser.add_argument('--unpacking', dest='unpacking', action='store_const', const=False,
                    default=False, help='Add PXD and SVD unpacking modules to the path')
parser.add_argument('--input-file', dest='input_file', action='store', default=None, type=str,
                    help='Name of imput file')
parser.add_argument('--filename-histos', dest='histo_file_name', action='store',
                    default='DQMPXDClusterShape_Histograms.root', type=str,
                    help='Name of output file with histograms in dqm option is used')

args = parser.parse_args()

"""
reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database('database.txt')
use_local_database(database_filename, "", LogLevel.DEBUG)
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)
use_local_database('database.txt')
use_local_database('localdb/database.txt')
use_local_database(Belle2.FileSystem.findFile("localdb/database.txt"))
"""
reset_database()
use_database_chain()
if args.global_tag is not None:
    use_central_database(args.global_tag, LogLevel.DEBUG)
    use_central_database("GT_gen_prod_003.01_Master-20170721-132500", LogLevel.DEBUG)
if args.local_db is not None:
    use_local_database(Belle2.FileSystem.findFile(args.local_db), "", LogLevel.DEBUG)
    use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
    use_local_database(Belle2.FileSystem.findFile("localdb/database.txt"))
# use_local_database(Belle2.FileSystem.findFile("centraldb/database.txt"))

# For Particle Gun:
CommonSets = {
    'nTracks': 1,
    'varyNTracks': False,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normal',
    'momentumParams': [2, 0.2],
    'phiGeneration': 'uniform',
    'phiParams': [0, 180],
    'thetaGeneration': 'uniformCos',
    'thetaParams': [1, 179],
    'xVertexGeneration': 'uniform',
    'xVertexParams': [-0.2050, -0.2],
    'yVertexGeneration': 'fixed',
    'zVertexGeneration': 'uniform',
    'independentVertices': False
}

branches = ['EventMetaData', 'RawFTSWs', 'RawSVDs', 'RawPXDs']
if (args.UseRealdata is True):
    # Limit branches use - necessary minimum (removes MC info if input is from simulation, HLT output)
    if not args.unpacking:
        branches = branches + ['PXDDigits', 'SVDDigits']
    if args.unpacking:
        PXDUnpacker = register_module('PXDUnpacker')
        PXDRawHitSorter = register_module('PXDRawHitSorter')
        SVDUnpacker = register_module('SVDUnpacker')
        SVDDigitSorter = register_module('SVDDigitSorter')
    rootinput = register_module('RootInput', branchNames=branches)
else:
    particlegun = register_module('ParticleGun')
    particlegun.param(CommonSets)
    simulation = register_module('FullSim')

histomanager = register_module('HistoManager', histoFileName=args.histo_file_name)
eventinfosetter = register_module('EventInfoSetter')
progress = register_module('Progress')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

PXDDIGI = register_module('PXDDigitizer')
PXDUnpacker = register_module('PXDUnpacker')
PXDRawHitSorter = register_module('PXDRawHitSorter')
PXDCLUST = register_module('PXDClusterizer')
# PXDCLUST = register_module('PXDClusterizer', notUseClusterShape=False)

SVDDIGI = register_module('SVDDigitizer')
SVDCLUST = register_module('SVDClusterizer')
SVDUnpacker = register_module('SVDUnpacker')
SVDDigitSorter = register_module('SVDDigitSorter')

CDCDigitizer = register_module('CDCDigitizer')

output = register_module('RootOutput')
output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size

# set_random_seed(1111123)
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
geometry.param('components', ['MagneticField', 'PXD', 'SVD', 'CDC'])

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO

# show warnings during processing
# set_log_level(LogLevel.WARNING)

ClShCorrectionFileName = "PXD_ClasterShapeCorrections.root"
ClShCorrectionLogFileName = "PXD_ClasterShapeCorrections.log"

ActivatePXDClusterShapeCorrection = register_module('ActivatePXDClusterShapeCorrection')
ActivatePXDClusterShapeCorrection.param('ClShCorrectionFileName', ClShCorrectionFileName)
ActivatePXDClusterShapeCorrection.param('ClShCorrectionLogFileName', ClShCorrectionLogFileName)

PXDDQMClusterShape = register_module('PXDDQMClusterShape')
PXDDQMClusterShape.param('CalFileName', ClShCorrectionFileName)
PXDDQMClusterShape.param('SeeDQMOfCalibration', 1)

# PXDDQMClusterShape.logging.log_level = LogLevel.DEBUG
PXDDQMClusterShape.logging.debug_level = 150
# PXDDQMClusterShape.set_log_level(LogLevel.INFO)
# PXDDQMClusterShape.set_log_level(LogLevel.DEBUG)
# PXDDQMClusterShape.set_debug_level(40)

# ============================================================================
# Do the DQM from the simulation or real data

main = create_path()
main.add_module(progress)

if (args.UseRealdata is True):
    # Limit branches use - necessary minimum (removes MC info if input is from simulation, HLT output)
    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(rootinput)
    if args.unpacking:
        main.add_module(PXDUnpacker)
        main.add_module(PXDRawHitSorter)
        main.add_module(SVDUnpacker)
        main.add_module(SVDDigitSorter)
    # main.add_module(simulation)
    # main.add_module(PXDDIGI)
    # main.add_module(SVDDIGI)
    # main.add_module(CDCDigitizer)
    # main.add_module(PXDCLUST)
    # main.add_module(SVDCLUST)
else:
    main.add_module(eventinfosetter)
    if (args.UseParticleGun is True):
        # generate simple particle gun events:
        main.add_module(particlegun)
    else:
        # generate BBbar events:
        main.add_module('EvtGenInput')
    add_simulation(main,
                   components=['MagneticField', 'PXD', 'SVD', 'CDC'],
                   usePXDDataReduction=False)

if (args.useShapeCorrection is True):
    main.add_module(ActivatePXDClusterShapeCorrection)

add_pxd_reconstruction(main)
add_svd_reconstruction(main)
add_tracking_reconstruction(main,
                            pruneTracks=False,
                            components=['MagneticField', 'PXD', 'SVD', 'CDC'],
                            additionalTrackFitHypotheses=[211, -211, 11, -11],
                            skipGeometryAdding=False,
                            reco_tracks="RecoTracks"
                            )

main.add_module(histomanager)
main.add_module(PXDDQMClusterShape)

# main.add_module("PrintCollections")
if (args.DataOutput is True):
    main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
#
