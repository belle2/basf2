#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run:
#   PXD, SVD, VXD and Track DQM on BelleII or Phase2 geometry
#   for ExpressReco on-line use Min modules without analysis
#   Base on module from Martin Ritter:
#               tracking/examples/DQMTracking_Phase2.py
# Contributors: Peter Kodys                                              *
#############################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from L1trigger import add_tsim
from daqdqm.commondqm import add_common_dqm
# import PXDDAQDQM

import PXDROIUnpackerModule
# import PXDHardwareClusterDQM
# import PXDHardwareClusterDQMModule

import glob

# background (collision) files
# bg = glob.glob('./BG/*.root')
# on KEKCC: (choose one of the sets)
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase2/set*/*.root
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase3/set*/*.root
bg = None

# number of events to generate, can be overriden with -n
num_events = 100
# output filename, can be overriden with -o
output_filename = "RootOutput_Phase3Early.root"

# create path
main = create_path()

# specify number of events to be generated
# main.add_module('EventInfoSetter', evtNumList=num_events)
# the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
# main.add_module("EventInfoSetter", expList=1002, runList=1, evtNumList=num_events)
main.add_module("EventInfoSetter", evtNumList=num_events)

# in case you need to fix seed of random numbers
# set_random_seed('some fixed value')
# set_random_seed('d33fa68eab781f3dcb069fb23425885fcd92d3432e6433a14894e5d7bba34272')

# generate BBbar events
main.add_module('EvtGenInput')

main.add_module("Gearbox", fileName='/geometry/Belle2_earlyPhase3.xml')
main.add_module("Geometry", useDB=False)


# detector simulation
add_simulation(main, bkgfiles=bg)

# trigger simulation
add_tsim(main)

# PXD digitization module
# main.add_module('PXDDigitizer')
# Convert digits to raw pxd data
PXDPACKER = register_module('PXDPacker')
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
main.add_module('HistoManager', histoFileName='Histos_DQMVXD_Phase3Early.root')
# main.add_module('HistoManager', histoFileName='Histos_DQMTracks_BelleII.root')

main.add_module('PXDDAQDQM', histogramDirectoryName='PXDDAQ')
main.add_module('PXDDQMClusters', histogramDirectoryName='PXDCls')
main.add_module('PXDDQMCorr', histgramDirectoryName='PXDCor')
main.add_module('PXDDQMEfficiency', histogramDirectoryName='PXDEff')
# main.add_module('PXDHardwareClusterDQM', histgramDirectoryName='PXDHCl')
main.add_module('PXDRawDQMChips', histogramDirectoryName='PXDRCh')
main.add_module('PXDRawDQMCorr', histgramDirectoryName='PXDRCo')
main.add_module('PXDRawDQM', histgramDirectoryName='PXDRaw')
main.add_module('PXDROIDQM', histgramDirectoryName='PXDROI')

pxddqmExpReco = register_module('PXDDQMExpressReco')
svddqmExpReco = register_module('SVDDQMExpressReco')
vxddqmExpReco = register_module('VXDDQMExpressReco')

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
process(main)
print(statistics)
