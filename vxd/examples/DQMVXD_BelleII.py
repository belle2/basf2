#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run DQM on BelleII geometry
# Valid for Phase 2, Phase 3 Early and Phase 3 regular as well as for testbeams
#   PXD, SVD, VXD and Track DQM
#   for ExpressReco on-line use Min modules without analysis
#   Base on module from Martin Ritter:
#               tracking/examples/DQMTracking_Phase2.py
# Contributors: Peter Kodys                                              *
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

import PXDROIUnpackerModule


import argparse
parser = argparse.ArgumentParser(
    description="VXD DQM Belle II for Phase 2 (Exp=1), Phase 3 Early (Exp=2) and Phase 3 regular (Exp=3), show all possible histos")
parser.add_argument('--experiment-type', dest='ExperimentType', action='store',
                    default=2, type=int,
                    help='Set which experiment you want: 1 (Phase 2), 2 (Phase 3 Early) or 3 (Phase 3 regular), default = 2')

args = parser.parse_args()

print("Final setting of arguments: ")
print("                 ExperimentType: ", args.ExperimentType)

# background (collision) files
# bg = glob.glob('./BG/*.root')
bg = None

# number of events to generate, can be overriden with -n
num_events = 100
# output filename, can be overriden with -o
output_filename = "RootOutput.root"
if (args.ExperimentType == 1):
    output_filename = "RootOutput_Phase2.root"
if (args.ExperimentType == 2):
    output_filename = "RootOutput_Phase3Early.root"
if (args.ExperimentType == 3):
    output_filename = "RootOutput_Phase3.root"

# create path
main = b2.create_path()

if (args.ExperimentType == 1):
    # the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
    main.add_module("EventInfoSetter", expList=1002, runList=1, evtNumList=num_events)
if (args.ExperimentType == 2):
    # the experiment number for early phase3 MC has to be 1003, otherwise the wrong payloads for this faze are loaded
    main.add_module("EventInfoSetter", expList=1003, runList=1, evtNumList=num_events)
if (args.ExperimentType == 3):
    # the experiment number for regular phase3 MC has no need to set, it is default
    main.add_module("EventInfoSetter", evtNumList=num_events)

# in case you need to fix seed of random numbers
# set_random_seed('d33fa68eab781f3dcb069fb23425885fcd92d3432e6433a14894e5d7bba34272')

# generate BBbar events
main.add_module('EvtGenInput')

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=bg)


# PXD digitization module
# main.add_module('PXDDigitizer')
# Convert digits to raw pxd data
PXDPACKER = b2.register_module('PXDPacker')
# ============================================================================
# [[dhhc1, dhh1, dhh2, dhh3, dhh4, dhh5] [ ... ]]
# -1 is disable port
PXDPACKER.param('dhe_to_dhc', [
    [0, 2, 4, 34, 36, 38],
    [1, 6, 8, 40, 42, 44],
    [2, 10, 12, 46, 48, 50],
    [3, 14, 16, 52, 54, 56],
    [4, 3, 5, 35, 37, 39],
    [5, 7, 9, 41, 43, 45],
    [6, 11, 13, 47, 49, 51],
    [7, 15, 17, 53, 55, 57],
])
main.add_module(PXDPACKER)
# Convert digits from raw pxd data
main.add_module('PXDUnpacker')
'''Unpack ROIs from ONSEN output'''
main.add_module(PXDROIUnpackerModule.PXDRawROIUnpackerModule())
'''Unpack ROIs from HLT Payload (depends if there are in the sroot file)'''
# main.add_module(PXDROIUnpackerModule.PXDPayloadROIUnpackerModule())

# reconstruction
add_reconstruction(main)

# histomanager: use DqmHistoManager for in-line monitoring, or HistoManager for offline training
# main.add_module('DqmHistoManager', Port=7777)
Histos_filename = "Histos_DQMVXD.root"
if (args.ExperimentType == 1):
    Histos_filename = "Histos_DQMVXD_Phase2.root"
if (args.ExperimentType == 2):
    Histos_filename = "Histos_DQMVXD_Phase3Early.root"
if (args.ExperimentType == 3):
    Histos_filename = "Histos_DQMVXD_Phase3.root"
main.add_module('HistoManager', histoFileName=Histos_filename)

main.add_module('PXDDAQDQM', histogramDirectoryName='PXDDAQ')
main.add_module('PXDDQMClusters', histogramDirectoryName='PXDCls')
main.add_module('PXDDQMCorr', histogramDirectoryName='PXDCor')
main.add_module('PXDDQMEfficiency', histogramDirectoryName='PXDEff')
# main.add_module('PXDHardwareClusterDQM', histogramDirectoryName='PXDHCl')
main.add_module('PXDRawDQMChips', histogramDirectoryName='PXDRCh')
main.add_module('PXDRawDQMCorr', histogramDirectoryName='PXDRCo')
main.add_module('PXDRawDQM', histogramDirectoryName='PXDRaw')
main.add_module('PXDROIDQM', histogramDirectoryName='PXDROI')

pxddqmExpReco = b2.register_module('PXDDQMExpressReco')
svddqmExpReco = b2.register_module('SVDDQMExpressReco')
vxddqmExpReco = b2.register_module('VXDDQMExpressReco')

main.add_module(pxddqmExpReco)
main.add_module(svddqmExpReco)
main.add_module(vxddqmExpReco)

# DQM of tracking
trackDQM = main.add_module('TrackDQM')
# In case to see more details:
# trackDQM = main.add_module('TrackDQM', debugLevel=250)
# trackDQM.logging.log_level = LogLevel.DEBUG

# Finally add output, if you need
# main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
