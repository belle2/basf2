#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run:
#   PXD and SVD masking on BelleII or Phase2 geometry
#   Base on module from Martin Ritter:
#               tracking/examples/DQMTracking_Phase2.py
# Contributors: Peter Kodys                                              *
#############################################################

from basf2 import *
from simulation import add_simulation
from L1trigger import add_tsim
import glob
import rawdata

from reconstruction import add_cosmics_reconstruction
from daqdqm.commondqm import add_common_dqm

# background (collision) files
# bg = glob.glob('./BG/*.root')
# on KEKCC: (choose one of the sets)
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase2/set*/*.root
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase3/set*/*.root
bg = None

# number of events to generate, can be overriden with -n
num_events = 1000
# output filename, can be overriden with -o
output_filename = "RootOutput_Phase2.root"

# in case you need to fix seed of random numbers
# set_random_seed('some fixed value')
# set_random_seed('d33fa68eab781f3dcb069fb23425885fcd92d3432e6433a14894e5d7bba34272')

# create path
main = create_path()

# REAL DATA:
main.add_module('RootInput')
# rawdata.add_unpackers(main, components=components)
main.add_module('Gearbox', fileName='/geometry/Beast2_phase2.xml')
main.add_module('Geometry', components=['PXD', 'SVD'])

# SIMULATIONS:
# specify number of events to be generated:
# main.add_module('EventInfoSetter', evtNumList=num_events)
# the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
# main.add_module("EventInfoSetter", expList=1002, runList=1, evtNumList=num_events)

# generate BBbar events:
# main.add_module('EvtGenInput')
# generate cosmics events:
# main.add_module('Cosmics')

# detector simulation:
# add_simulation(main, bkgfiles=bg)

# trigger simulation:
# add_tsim(main)

# histomanager: use DqmHistoManager for in-line monitoring, or HistoManager for offline training
# main.add_module('DqmHistoManager', Port=7777)
main.add_module('HistoManager', histoFileName='Histos_DQMTracks_Phase2.root')
# main.add_module('HistoManager', histoFileName='Histos_DQMTracks_BelleII.root')

# just as dummy, yous real one parced from file:
RunNoVXD = 2002002

pxd_maskBasic = 'PXD_MaskFiredBasic.xml'
svd_maskBasic = 'SVD_MaskFiredBasic.xml'
pxd_mask = 'PXD_MaskFired_Run'+str(RunNoVXD)+'.xml'
svd_mask = 'SVD_MaskFired_Run'+str(RunNoVXD)+'.xml'

Mask = register_module('vxdDigitMasking')
Mask.param('nEventsProcess', 1000)
# Mask.param('AppendMaskFile', 1)
# Mask.param('PXDCut', 50)
# Mask.param('SVDuCut', 50)
# Mask.param('SVDvCut', 50)
# Mask.param('PXDChargeCut', 10)
# Mask.param('SVDuChargeCut', 10)
# Mask.param('SVDvChargeCut', 10)
Mask.param('PXDMaskFileBasicName', pxd_maskBasic)
Mask.param('SVDMaskFileBasicName', svd_maskBasic)
Mask.param('PXDMaskFileRunName', pxd_mask)
Mask.param('SVDMaskFileRunName', svd_mask)

main.add_module(Mask)


# main.add_module('PXDDQMExpressReco')
# main.add_module('SVDDQMExpressRecoMin')

# Finally add output, if you need
# main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
process(main)
print(statistics)
