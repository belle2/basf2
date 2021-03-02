#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Simple steering file to demonstrate how to run Track DQM on BelleII geometry
# Valid for Phase 2, Phase 3 Early and Phase 3 regular as well as for testbeams
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from L1trigger import add_tsim

import argparse
parser = argparse.ArgumentParser(
    description="Tracking DQM Belle II for Phase 2 (Exp=1), Phase 3 Early (Exp=2) and Phase 3 regular (Exp=3)")
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

# detector simulation
add_simulation(main, bkgfiles=bg)

# trigger simulation
add_tsim(main)

# reconstruction
add_reconstruction(main)

# histomanager: use DqmHistoManager for in-line monitoring, or HistoManager for offline training
# main.add_module('DqmHistoManager', Port=7777)
Histos_filename = "Histos_DQMTracks.root"
if (args.ExperimentType == 1):
    Histos_filename = "Histos_DQMTracks_Phase2.root"
if (args.ExperimentType == 2):
    Histos_filename = "Histos_DQMTracks_Phase3Early.root"
if (args.ExperimentType == 3):
    Histos_filename = "Histos_DQMTracks_Phase3.root"
main.add_module('HistoManager', histoFileName=Histos_filename)

# DQM of tracking
trackDQM = main.add_module('TrackDQM', debugLevel=250)
# In case to see more details:
# trackDQM.logging.log_level = LogLevel.DEBUG

# Finally add output, if you need
# main.add_module("RootOutput", outputFileName=output_filename)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
