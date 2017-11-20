# This steering file shows pretty much the most minimal setup for
# running the CAF. You will need to have data already from running
# calibration/examples/1_create_sample_DSTs.sh or just make your own
# and change the input data below.

from basf2 import *
set_log_level(LogLevel.INFO)
# add time stamp to all INFO messages
# currentInfo = logging.get_info(LogLevel.INFO)
# logging.set_info(LogLevel.INFO, currentInfo | LogInfo.TIMESTAMP)

import os
import sys

import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf.utils import IoV


def main(argv):
    if len(argv) == 1:
        data_dir = argv[0]
    else:
        print("Usage: basf2 CAF_simplest.py <data directory>")
        sys.exit(1)

    ###################################################
    # Input Data
    # This part assumes that you've created the data using the calibration/examples/1_create_sample_DSTs.sh
    # We'll use the same data for all calibrations but this is not a requirement in general.
    # ALWAYS USE ABSOLUTE PATHS TO THE FILES! i.e. remember to os.path.abspath(file) them
    input_files_test = []
    input_files_test.append(os.path.join(os.path.abspath(data_dir), '*.root'))

    ###################################################
    # Test Calibration Setup
    alg_test = TestCalibrationAlgorithm()  # Getting a calibration algorithm instance

    # Create a single calibration from a collector module name + algorithm + input files
    cal_test = Calibration(name="TestCalibration", collector="CaTest", algorithms=alg_test, input_files=input_files_test)

    ###################################################
    # Create a CAF instance and add the calibration to it. Should run on one CPU core locally by default.
    cal_fw = CAF()
    cal_fw.add_calibration(cal_test)

    # Let's only calibrate a subset of the data
    iov_to_calibrate = IoV(exp_low=1, run_low=2, exp_high=1, run_high=3)
    cal_fw.run(iov=iov_to_calibrate)
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
