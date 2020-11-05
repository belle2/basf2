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


conditions.prepend_globaltag("online")
conditions.prepend_globaltag("svd_basic")
conditions.prepend_globaltag("svd_loadedOnFADC")

myLocalDB = None

if myLocalDB is not None:
    conditions.testing_payloads = [str(myLocalDB)]
else:
    B2INFO("No local DB provided, monitoring payloads from GTs.")

if myLocalDB is not None:
    filenameLocal = "SVDLocalCalibrationMonitor_experiment" + \
        str(ExpList[0]) + "_run" + str(RunList[0]) + "_fromLocalDB"+str(localDB_tag)+".root"
    filenameCoG = "SVDCoGTimeCalibrationMonitor_experiment" + \
        str(ExpList[0]) + "_run" + str(RunList[0]) + "_fromLocalDB"+str(localDB_tag)+".root"
    filenameCluster = "SVDClusterCalibrationMonitor_experiment" + \
        str(ExpList[0]) + "_run" + str(RunList[0]) + "_fromLocalDB"+str(localDB_tag)+".root"
else:
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
