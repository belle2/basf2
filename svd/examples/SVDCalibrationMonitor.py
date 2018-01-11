#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Calibration Monitor.
Simple example for testing the SVDCalibrationMonitorModule
"""

from basf2 import *
from svd.testbeam_utils import *
import ROOT

import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

use_central_database("beamtest_vxd_april2017_rev1")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': [3], 'runList': [400]})
# eventinfosetter.param({'evtNumList': [1], 'expList': [3], 'runList': [111]})
main.add_module(eventinfosetter)

# Generate an event
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# define geometry version
main.add_module('Gearbox')
# main.add_module('Geometry')
add_geometry(main, magnet=True, field_override=None, target=None, geometry_version=1)
# add SVD calibration module
svdcalibmonitor = register_module('SVDCalibrationsMonitor')
svdcalibmonitor. param('outputFileName', "SVDCalibrationMonitor_TB400.root")
main.add_module(svdcalibmonitor)

# process single event
process(main)
