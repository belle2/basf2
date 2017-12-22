#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Calibration Monitor.
Simple example for testing the SVDCalibrationMonitorModule
"""

from basf2 import *
import ROOT

import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# Generate an event
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# define geometry version
main.add_module('Gearbox')
main.add_module('Geometry')

# add SVD calibration module
svdcalibmonitor = register_module('SVDCalibrationsMonitor')
svdcalibmonitor. param('outputFileName', "SVDCalibrationMonitor_test.root")
main.add_module(svdcalibmonitor)

# process single event
process(main)
