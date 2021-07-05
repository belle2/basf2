#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

# background (collision) files
# bg = glob.glob('./BG/*.root')
# on KEKCC: (choose one of the sets)
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase2/set*/*.root
# bg = /group/belle2/BGFile/OfficialBKG/15thCampaign/phase3/set*/*.root
bg = None

# number of events to generate, can be overriden with -n
num_events = 100
# output filename, can be overriden with -o
output_filename = "RootOutput_Phase2.root"

# create path
main = b2.create_path()

# specify number of events to be generated
# main.add_module('EventInfoSetter', evtNumList=num_events)
# the experiment number for phase2 MC has to be 1002, otherwise the wrong payloads (for VXDTF2 the SectorMap) are loaded
main.add_module("EventInfoSetter", expList=1002, runList=1, evtNumList=num_events)

# in case you need to fix seed of random numbers
# set_random_seed('some fixed value')
# set_random_seed('d33fa68eab781f3dcb069fb23425885fcd92d3432e6433a14894e5d7bba34272')

# generate BBbar events
main.add_module('EvtGenInput')

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=bg)

# reconstruction
add_reconstruction(main)

# histomanager: use DqmHistoManager for in-line monitoring, or HistoManager for offline training
# main.add_module('DqmHistoManager', Port=7777)
main.add_module('HistoManager', histoFileName='Histos_DQMTracks_Phase2.root')
# main.add_module('HistoManager', histoFileName='Histos_DQMTracks_BelleII.root')

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
