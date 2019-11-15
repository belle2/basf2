#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Local Calibration Monitor Script
usage:
> open this file and edit the file:
. GLOBAL_TAG = xxx
. you can add also a local database uncommenting the relative line
Then execute the script:
> basf2 SVDLocalCalibrationMonitor.py --exp EXP --run RUN --local --cluster --cog
where:
local -> local calibration (from xml)
cluster -> cluster parameters
cog -> cog calibration parameters
"""

from basf2 import *
import ROOT
import argparse
from basf2 import conditions


parser = argparse.ArgumentParser(description="SVD Calibration Monitor")
parser.add_argument('--exp', metavar='expNumber', dest='exp', type=int, nargs=1, help='Experiment Number')
parser.add_argument('--run', metavar='runNumber', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--local', dest='doLocal', action='store_const', const=True,
                    default=False, help='produce Local Calibration Monitor plots')
parser.add_argument('--cog', dest='doCoG', action='store_const', const=True,
                    default=False, help='produce CoG Calibration Monitor plots')
parser.add_argument('--cluster', dest='doCluster', action='store_const', const=True,
                    default=False, help='produce Cluster Calibration Monitor plots')
parser.print_help()
print('')
args = parser.parse_args()
RunList = args.run
ExpList = args.exp

# check the global tag first:
# GLOBAL_TAG = "svd_Belle2_20181221"


conditions.override_globaltags()
conditions.globaltags = [
    "data_reprocessing_proc10",
    "data_reprocessing_prompt_rel4_patchb",
    "giulia_CDCEDepToADCConversions_rel4_patch"]

myLocalDB = "run"+str(RunList[0]) + "/calibration_results/SVDOccupancyAndHotStrips/outputdb/database.txt"
print('Your are plotting occupancy from payloads belongin to the local DB:')
print('')
print(myLocalDB)

conditions.testing_payloads = [str(myLocalDB)]


# TO BE USED before release 04, with previous database version:
# reset_database()
# use_database_chain()
# uncomment if using a local database:
# use_local_database(str(myLocalDB)+"database.txt", str(myLocalDB), invertLogging=True)
# use_local_database("localDB/database.txt", "localDB", invertLogging=True)
# use_central_database(GLOBAL_TAG)

filenameLocal = "SVDLocalCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"
filenameCoG = "SVDCoGTimeCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"
filenameCluster = "SVDClusterCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': ExpList, 'runList': RunList})
main.add_module(eventinfosetter)
main.add_module("Gearbox")
main.add_module("Geometry")

# add calibration monitor modules


if args.doLocal:
    local = register_module('SVDLocalCalibrationsMonitor')
    local. param('outputFileName', filenameLocal)
    main.add_module(local)

if args.doCoG:
    cog = register_module('SVDCoGTimeCalibrationsMonitor')
    cog. param('outputFileName', filenameCoG)
    main.add_module(cog)

if args.doCluster:
    cluster = register_module('SVDClusterCalibrationsMonitor')
    cluster. param('outputFileName', filenameCluster)
    main.add_module(cluster)

# process single event
print_path(main)
process(main)
