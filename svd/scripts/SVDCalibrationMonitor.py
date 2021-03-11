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

import basf2 as b2
import argparse


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


b2.conditions.prepend_globaltag("online")
b2.conditions.prepend_globaltag("svd_basic")
b2.conditions.prepend_globaltag("svd_loadedOnFADC")

myLocalDB = None

if myLocalDB is not None:
    b2.conditions.testing_payloads = [str(myLocalDB)]
else:
    b2.B2INFO("No local DB provided, monitoring payloads from GTs.")

if myLocalDB is not None:
    filenameLocal = "SVDLocalCalibrationMonitor_experiment" + \
        str(ExpList[0]) + "_run" + str(RunList[0]) + "_fromLocalDB" + str(myLocalDB) + ".root"
    filenameCoG = "SVDCoGTimeCalibrationMonitor_experiment" + \
        str(ExpList[0]) + "_run" + str(RunList[0]) + "_fromLocalDB" + str(myLocalDB) + ".root"
    filenameCluster = "SVDClusterCalibrationMonitor_experiment" + \
        str(ExpList[0]) + "_run" + str(RunList[0]) + "_fromLocalDB" + str(myLocalDB) + ".root"
else:
    filenameLocal = "SVDLocalCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"
    filenameCoG = "SVDCoGTimeCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"
    filenameCluster = "SVDClusterCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"

main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': ExpList, 'runList': RunList})
main.add_module(eventinfosetter)
main.add_module("Gearbox")
main.add_module("Geometry")

# add calibration monitor modules
if args.doLocal:
    local = b2.register_module('SVDLocalCalibrationsMonitor')
    local. param('outputFileName', filenameLocal)
    main.add_module(local)

if args.doCoG:
    cog = b2.register_module('SVDCoGTimeCalibrationsMonitor')
    cog. param('outputFileName', filenameCoG)
    main.add_module(cog)

if args.doCluster:
    cluster = b2.register_module('SVDClusterCalibrationsMonitor')
    cluster. param('outputFileName', filenameCluster)
    main.add_module(cluster)

# process single event
b2.print_path(main)
b2.process(main)
