#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Calibration Monitor.
Simple example for testing the SVDCalibrationMonitorModule
"""

from basf2 import *
# only used for TB calibration monitoring
# from svd.testbeam_utils import *
import ROOT

import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

# use_central_database("beamtest_vxd_april2017_rev1", loglevel=LogLevel.DEBUG)
use_central_database("GT_gen_prod_004.05_prerelease-01-00-00b")
# use_local_database("/home/belle2/zani/HEAD/localDB_run111_fixedTime/database_run111_fixedTime.txt")
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
# eventinfosetter.param({'evtNumList': [1], 'expList': [3], 'runList': [400]})
# eventinfosetter.param({'evtNumList': [1], 'expList': [3], 'runList': [111]})
eventinfosetter.param({'evtNumList': [1], 'expList': [1], 'runList': [1]})
main.add_module(eventinfosetter)

# Generate an event
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# only fro TB calibrations monitor, define geometry version

main.add_module('Gearbox')
main.add_module('Geometry')

'''
add_geometry(main, magnet=True, field_override=None, target=None, geometry_version=1)
'''

# add SVD calibration module
svdcalibmonitor = register_module('SVDCalibrationsMonitor')
svdcalibmonitor. param('outputFileName', "SVDCalibrationMonitor_phase3_testBeforePull.root")
main.add_module(svdcalibmonitor)

# process single event
process(main)
