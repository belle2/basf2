#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Calibration Monitor.
Simple example for testing the SVDCalibrationMonitorModule
"""

from basf2 import *
from svd.testbeam_utils import *
import ROOT

import argparse
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

parser = argparse.ArgumentParser(description="SVD Calibration Monitor")

# 2017 testbeam
parser.add_argument('--TB-magnet-on', dest='TB_magnet_on', action='store_const', const=True, default=False, help='testbeam run 400')
parser.add_argument(
    '--TB-magnet-off',
    dest='TB_magnet_off',
    action='store_const',
    const=True,
    default=False,
    help='testbeam run 111')

# data
parser.add_argument('--experiment', metavar='expNumber', dest='exp', type=int, nargs=1, help='Experiment Number')
parser.add_argument('--run', metavar='runNumber', dest='run', type=int, nargs=1, help='Run Number')

parser.print_help()
print('')

args = parser.parse_args()

if(args.TB_magnet_on or args.TB_magnet_off):
    use_central_database("beamtest_vxd_april2017_rev1")
    ExpList = [3]

    if(args.TB_magnet_off):
        RunList = [111]
        filename = "SVDCalibrationMonitor_2017TB_experiment3_run111.root"
        geom = 0
    else:
        RunList = [400]
        filename = "SVDCalibrationMonitor_2017TB_experiment3_run400.root"
        geom = 1
else:
    RunList = args.run
    ExpList = args.exp
    filename = "SVDCalibrationMonitor_experiment" + str(ExpList[0]) + "_run" + str(RunList[0]) + ".root"


main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': ExpList, 'runList': RunList})
main.add_module(eventinfosetter)

# main.add_module('EvtGenInput')

main.add_module('Gearbox')
if(args.TB_magnet_on or args.TB_magnet_off):
    add_geometry(main, magnet=True, field_override=None, target=None, geometry_version=geom)
else:
    geometry = register_module('Geometry', useDB=True)
    main.add_module(geometry)

# add SVD calibration module
svdcalibmonitor = register_module('SVDCalibrationsMonitor')
svdcalibmonitor. param('outputFileName', filename)
main.add_module(svdcalibmonitor)

# process single event
process(main)
