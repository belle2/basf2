#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Local Calibration Monitor Script
usage:
> open this file and edit the file:
. GLOBAL_TAG = xxx
. you can add also a local database uncommenting the relative line
Then execute the script:
> basf2 SVDLocalCalibrationMonitor.py --exp EXP --run RUN
"""

from basf2 import *
import ROOT

import argparse

parser = argparse.ArgumentParser(description="SVD Calibration Monitor")
parser.add_argument('--exp', metavar='expNumber', dest='exp', type=int, nargs=1, help='Experiment Number')
parser.add_argument('--run', metavar='runNumber', dest='run', type=int, nargs=1, help='Run Number')
parser.print_help()
print('')
args = parser.parse_args()

# check the global tag first:
GLOBAL_TAG = "data_reprocessing_proc7"

reset_database()
use_database_chain()
# uncomment if using a local database:
# use_local_database("localDB/database.txt","localDB",invertLogging=True)
use_central_database(GLOBAL_TAG)
RunList = args.run
ExpList = args.exp
filename = "SVDLocalCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"


main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': ExpList, 'runList': RunList})
main.add_module(eventinfosetter)
main.add_module("Gearbox")
main.add_module("Geometry")

# add SVD calibration module
svdcalibmonitor = register_module('SVDLocalCalibrationsMonitor')
svdcalibmonitor. param('outputFileName', filename)
main.add_module(svdcalibmonitor)

# process single event
process(main)
