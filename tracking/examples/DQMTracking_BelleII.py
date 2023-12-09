#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# Simple steering file to demonstrate how to run Track DQM on BelleII geometry
# Valid for early phase 3 and nominal phase 3
#############################################################

import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

import argparse
parser = argparse.ArgumentParser(
    description="Tracking DQM Belle II for early phase 3 (Exp=1) and nominal phase 3 (Exp=2)")
parser.add_argument('--experiment-type', dest='ExperimentType', action='store',
                    default=2, type=int,
                    help='Set which experiment you want: 1 (early phase 3) or 2 (nominal phase 3), default = 2')

args = parser.parse_args()

if args.ExperimentType not in [1, 2]:
    import sys
    print("Parameter experiment-type must be either 1 (early phase 3) "
          f"or 2 (nominal phase 3, default), but you set {args.ExperimentType}. Exiting.")
    sys.exit(1)

# number of events to generate, can be overriden with -n
num_events = 100

# create path
main = b2.create_path()

if (args.ExperimentType == 1):
    # the experiment number for early phase3 MC has to be 1003, otherwise the wrong payloads for this faze are loaded
    main.add_module("EventInfoSetter", expList=1003, runList=1, evtNumList=num_events)
elif (args.ExperimentType == 2):
    # the experiment number for nominal phase3 MC has no need to set, it is default
    main.add_module("EventInfoSetter", evtNumList=num_events)

# Set a random seed to ensure reproducibility
b2.set_random_seed('some_seed')

# generate BBbar events
main.add_module('EvtGenInput')

# detector and L1 trigger simulation
add_simulation(main, bkgfiles=None)

# reconstruction
add_reconstruction(main)

# histomanager: use DqmHistoManager for in-line monitoring, or HistoManager for offline training
# main.add_module('DqmHistoManager', Port=7777)
Histos_filename = ""
if (args.ExperimentType == 1):
    Histos_filename = "Histos_DQMTracks_Phase3Early.root"
elif (args.ExperimentType == 2):
    Histos_filename = "Histos_DQMTracks_Phase3.root"
main.add_module('HistoManager', histoFileName=Histos_filename)

# DQM of tracking
trackDQM = main.add_module('TrackDQM', debugLevel=250)
# In case to see more details:
# trackDQM.logging.log_level = LogLevel.DEBUG

# output filename, can be overriden with -o
output_filename = ""
if (args.ExperimentType == 1):
    output_filename = "RootOutput_Phase3Early.root"
elif (args.ExperimentType == 2):
    output_filename = "RootOutput_Phase3.root"
main.add_module("RootOutput", outputFileName=output_filename)

main.add_module('Progress')

# process events and print call statistics
b2.process(main)
print(b2.statistics)
