#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file shows pretty much the most minimal setup for
# running the CAF. You will need to have data already from running
# calibration/examples/1_create_sample_DSTs.sh or just make your own
# and change the input data below.

import basf2 as b2

import os

from ROOT.Belle2 import TestDBAccessAlgorithm
from caf.framework import Calibration, CAF
from caf.utils import IoV
from caf.strategies import SequentialRunByRun

b2.set_log_level(b2.LogLevel.DEBUG)
b2.set_debug_level(100)
# add time stamp to all INFO messages
# currentInfo = logging.get_info(LogLevel.INFO)
# logging.set_info(LogLevel.INFO, currentInfo | LogInfo.TIMESTAMP)
data_dir = "../../../examples/test_data"

###################################################
# Input Data
# This part assumes that you've created the data using the calibration/examples/1_create_sample_DSTs.sh
# We'll use the same data for all calibrations but this is not a requirement in general.
# ALWAYS USE ABSOLUTE PATHS TO THE FILES! i.e. remember to os.path.abspath(file) them
input_files_test = []
input_files_test.append(os.path.join(os.path.abspath(data_dir), '*.root'))

###################################################
# Test Calibration Setup
alg_test = TestDBAccessAlgorithm()  # Getting a calibration algorithm instance
# alg_test.setMinEntries(15000)

col = b2.register_module("CaTest")
# col.param("granularity", "all")

# Create a single calibration from a collector module name + algorithm + input files
cal_test = Calibration(name="TestCalibration", collector="CaTest", algorithms=alg_test, input_files=input_files_test)
cal_test.strategies = SequentialRunByRun
###################################################
# Create a CAF instance and add the calibration to it. Should run on one CPU core locally by default.
cal_fw = CAF()
cal_fw.add_calibration(cal_test)

# Let's only calibrate a subset of the data
iov_to_calibrate = IoV(exp_low=0, run_low=1, exp_high=0, run_high=4)
# cal_fw.run(iov=iov_to_calibrate)
cal_fw.run()
print("End of CAF processing.")
