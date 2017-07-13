#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# This steering file to see DQM - basic of VXD sensors
# and correlations
#
# Sample of external file, 100 particlegun or BBbar events.
#                    (default is particlegun)
#
# Usage: basf2 DQMCorrelations.py
#      ( basf2 DQMCorrelations.py -- --h )
#
# Input: external file or None (simulation)
# Output: histograms: VXD_DQM_Histograms.root or custom...
#
# There are prepared 3 levels of DQM histograms:
#    1. ExpressReco, very restricted, for on-line monitor
#    2. Chip granulation, need higher statistics, for on-line
#    3. Details, as much as possible details, for off-line
# Default is to show all possible histograms
#
# Contributors: Peter Kodys                                              *
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
from ROOT import Belle2

import argparse
parser = argparse.ArgumentParser(description="PXD+SVD+VXD DQM correlations for Belle II + TB, show all possible histos")
parser.add_argument('--local-db', dest='local_db', action='store', default=None, type=str, help='Location of local db')
parser.add_argument(
    '--global-tag',
    dest='global_tag',
    action='store',
    default=None,
    type=str,
    help='Global tag to use at central DB in PNNL')
parser.add_argument(
    '--magnet-off',
    dest='magnet_off',
    action='store_const',
    const=True,
    default=False,
    help='Turn off magnetic field')

parser.add_argument('--skip-pxd-svd', dest='SkipPXDSVD', action='store_const', const=True, default=False,
                    help='Add also PXD and SVD DQM, default = True')
parser.add_argument('--data-output', dest='DataOutput', action='store_const', const=True, default=False,
                    help='Save data to output file, default = False')
parser.add_argument('--correlation-granulation', dest='CorrelationGranulation', action='store', default=1.0, type=float,
                    help='Set granulation of histogram plots, default is 1 degree, min = 0.02 degree, max = 1 degree')
parser.add_argument('--cut-correlation-sig-pxd', dest='CutCorrelationSigPXD', action='store', default=0, type=int,
                    help='Cut threshold of PXD signal for accepting to correlations, default = 0 ADU')
parser.add_argument('--cut-correlation-sig-u-svd', dest='CutCorrelationSigUSVD', action='store', default=0, type=int,
                    help='Cut threshold of SVD signal for accepting to correlations in u, default = 0 ADU')
parser.add_argument('--cut-correlation-sig-v-svd', dest='CutCorrelationSigVSVD', action='store', default=0, type=int,
                    help='Cut threshold of SVD signal for accepting to correlations in v, default = 0 ADU')
parser.add_argument('--cut-correlation-time-svd', dest='CutCorrelationTimeSVD', action='store', default=70, type=int,
                    help='Cut threshold of SVD time window for accepting to correlations, default = 70 ns')

parser.add_argument('--use-digits', dest='UseDigits', action='store', default=0, type=int,
                    help='flag <0,1> for using digits only, no cluster information will be required, default = 0')
parser.add_argument('--reduce-1d-correl-histos', dest='Reduce1DCorrelHistos', action='store', default=0, type=int,
                    help='flag <0,1> for removing of 1D correlation plots from output, default = 0')
parser.add_argument('--reduce-2d-correl-histos', dest='Reduce2DCorrelHistos', action='store', default=0, type=int,
                    help='flag <0,1> for removing of 2D correlation plots from output, default = 0')
parser.add_argument('--only-23-layers-histos', dest='Only23LayersHistos', action='store', default=0, type=int,
                    help='flag <0,1> for to keep only correlation plots between layer 2 and 3 (between PXD and SVD), default = 0')
parser.add_argument('--save-other-histos', dest='SaveOtherHistos', action='store', default=1, type=int,
                    help='flag <0,1> for creation of correlation plots for non-neighboar layers, default = 0')
parser.add_argument('--use-real-data', dest='UseRealdata', action='store_const', const=False,
                    default=False, help='Use real data, need set imput file, unpacking')
parser.add_argument('--unpacking', dest='unpacking', action='store_const', const=True,
                    default=False, help='Add PXD and SVD unpacking modules to the path')
parser.add_argument('--input-file', dest='input_file', action='store', default=None, type=str,
                    help='Name of input file')
parser.add_argument('--filename-histos', dest='histo_file_name', action='store',
                    default='VXD_DQM_Histograms.root', type=str,
                    help='Name of output file with histograms in dqm option is used')
parser.add_argument('--SkipDQM', dest='SkipDQM', action='store_const', const=True, default=False, help='Produce DQM plots')
parser.add_argument('--SkipDQMExpressReco', dest='SkipDQMExpressReco', action='store_const', const=True, default=False,
                    help='Skip production of ExpressReco DQM plots')
parser.add_argument('--SkipDQMChips', dest='SkipDQMChips', action='store_const', const=True, default=False,
                    help='Skip production of chips DQM plots')
parser.add_argument('--SkipDQMDetail', dest='SkipDQMDetail', action='store_const', const=True, default=False,
                    help='Skip production of detail DQM plots')
parser.add_argument('--CreateDB', dest='CreateDB', action='store', default=0, type=int,
                    help='Create DataBase references for SVD DQM Express Reco, default = 0')

args = parser.parse_args()

print("Final setting of arguments: ")
print("                 SkipPXDSVD: ", args.SkipPXDSVD)
print("                 DataOutput: ", args.DataOutput)
print("     CorrelationGranulation: ", args.CorrelationGranulation)
print("       CutCorrelationSigPXD: ", args.CutCorrelationSigPXD)
print("      CutCorrelationSigUSVD: ", args.CutCorrelationSigUSVD)
print("      CutCorrelationSigVSVD: ", args.CutCorrelationSigVSVD)
print("      CutCorrelationTimeSVD: ", args.CutCorrelationTimeSVD)
print("                  UseDigits: ", args.UseDigits)
print("       Reduce1DCorrelHistos: ", args.Reduce1DCorrelHistos)
print("       Reduce2DCorrelHistos: ", args.Reduce2DCorrelHistos)
print("         Only23LayersHistos: ", args.Only23LayersHistos)
print("            SaveOtherHistos: ", args.SaveOtherHistos)
print("                UseRealdata: ", args.UseRealdata)
print("                  unpacking: ", args.unpacking)
print("                 input_file: ", args.input_file)
print("            histo_file_name: ", args.histo_file_name)
print("                    SkipDQM: ", args.SkipDQM)
print("         SkipDQMExpressReco: ", args.SkipDQMExpressReco)
print("               SkipDQMChips: ", args.SkipDQMChips)
print("              SkipDQMDetail: ", args.SkipDQMDetail)
print("                   CreateDB: ", args.CreateDB)

# setup_database(args.local_db, args.global_tag)
# if (args.CreateDB == 0):
# reset_database()
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True, LogLevel.ERROR)

if (args.UseRealdata is True):
    # Limit branches use - necessary minimum (removes MC info if input is from simulation, HLT output)
    branches = ['EventMetaData', 'RawFTSWs', 'RawSVDs', 'RawPXDs']
    if not args.unpacking:
        branches = branches + ['PXDDigits', 'SVDDigits']
    rootinput = register_module('RootInput', branchNames=branches)
else:
    particlegun = register_module('ParticleGun')
    simulation = register_module('FullSim')

histomanager = register_module('HistoManager', histoFileName=args.histo_file_name)

eventinfosetter = register_module('EventInfoSetter')
progress = register_module('Progress')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
PXDDIGI = register_module('PXDDigitizer')
PXDCLUST = register_module('PXDClusterizer')
SVDDIGI = register_module('SVDDigitizer')
SVDCLUST = register_module('SVDClusterizer')

PXDUnpacker = register_module('PXDUnpacker')
PXDRawHitSorter = register_module('PXDRawHitSorter')
SVDUnpacker = register_module('SVDUnpacker')
SVDDigitSorter = register_module('SVDDigitSorter')

output = register_module('RootOutput')
# output.param('branchNames', ['EventMetaData'])  # cannot be removed, but of only small effect on file size

# set_random_seed(1028307)
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
if (args.magnet_off is True):
    geometry.param('components', ['PXD', 'SVD'])
else:
    geometry.param('components', ['MagneticField', 'PXD', 'SVD'])

if (args.SkipPXDSVD is False):
    pxddqmExpReco = register_module('PXDDQMExpressReco')
    pxddqmExpReco.param('CreateDB', args.CreateDB)
    pxddqmChips = register_module('PXDDQM')
    pxddqmDetails = register_module('PXDDQM')
    # pxddqm.param('UseDigits', args.UseDigits)
    # pxddqm.param('SaveOtherHistos', args.SaveOtherHistos)

    svddqmExpReco = register_module('SVDDQMExpressReco')
    svddqmExpReco.param('CreateDB', args.CreateDB)
    svddqmChips = register_module('SVDDQM')
    svddqmDetails = register_module('SVDDQM')
    # svddqm.param('UseDigits', args.UseDigits)
    # svddqm.param('SaveOtherHistos', args.SaveOtherHistos)

param_vxddqm1 = {'CutCorrelationSigPXD': args.CutCorrelationSigPXD,
                 'CutCorrelationSigUSVD': args.CutCorrelationSigUSVD,
                 'CutCorrelationSigVSVD': args.CutCorrelationSigVSVD,
                 'CutCorrelationTimeSVD': args.CutCorrelationTimeSVD,
                 'UseDigits': args.UseDigits
                 }
param_vxddqm2 = {'CorrelationGranulation': args.CorrelationGranulation,
                 'Reduce1DCorrelHistos': args.Reduce1DCorrelHistos,
                 'Reduce2DCorrelHistos': args.Reduce2DCorrelHistos,
                 'Only23LayersHistos': args.Only23LayersHistos,
                 'SaveOtherHistos': args.SaveOtherHistos,
                 'SwapPXD': 0
                 }

vxddqmExpReco = register_module('VXDDQMExpressReco')
vxddqmExpReco.param('CreateDB', args.CreateDB)
vxddqmChips = register_module('VXDDQM')
vxddqmDetails = register_module('VXDDQM')

vxddqmExpReco.param(param_vxddqm1)
vxddqmChips.param(param_vxddqm1)
vxddqmDetails.param(param_vxddqm1)
vxddqmDetails.param(param_vxddqm2)

eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

main = create_path()
if (args.UseRealdata is False):
    main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
if (args.UseRealdata is True):
    if args.unpacking:
        PXDUnpacker = register_module('PXDUnpacker')
        PXDRawHitSorter = register_module('PXDRawHitSorter')
        SVDUnpacker = register_module('SVDUnpacker')
        SVDDigitSorter = register_module('SVDDigitSorter')
    main.add_module(rootinput)
else:
    # generate simple particle gun events:
    main.add_module(particlegun)

    # generate BBbar events:
    # main.add_module('EvtGenInput')

    main.add_module(simulation)

main.add_module(PXDDIGI)
main.add_module(SVDDIGI)
if (args.UseDigits == 0):
    main.add_module(PXDCLUST)
    main.add_module(SVDCLUST)

if (args.SkipDQM is False):
    main.add_module(histomanager)
    if (args.SkipPXDSVD is False):
        if (args.SkipDQMExpressReco is False):
            main.add_module(pxddqmExpReco)
            main.add_module(svddqmExpReco)
#        if (args.SkipDQMChips is False):
#            main.add_module(pxddqmChips)
#            main.add_module(svddqmChips)
#        if (args.SkipDQMDetail is False):
#            main.add_module(pxddqmDetails)
#            main.add_module(svddqmDetails)
    if (args.SkipDQMExpressReco is False):
        main.add_module(vxddqmExpReco)
#    if (args.SkipDQMChips is False):
#        main.add_module(vxddqmChips)
#    if (args.SkipDQMDetail is False):
#        main.add_module(vxddqmDetails)

# main.add_module("PrintCollections")
if (args.DataOutput is True):
    main.add_module(output)

# Process events
process(main)

# Print call statistics
print(statistics)
#
