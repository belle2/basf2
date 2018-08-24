# This steering file shows off some more options that can be configured
# and how to run multiple dependent calibrations. You will need to have
# data already from running calibration/examples/1_create_sample_DSTs.sh
# or just make your own any change the input data below.

from basf2 import *
# set_log_level(LogLevel.DEBUG)
# set_debug_level(29)
set_log_level(LogLevel.INFO)

import os
import sys

import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm

from caf.framework import Calibration, CAF
from caf import backends
from caf.utils import ExpRun


def main(argv):
    if len(argv) == 1:
        data_dir = argv[0]
    else:
        print("Usage: python3 caf_ignoring_runs.py <data directory>")
        sys.exit(1)

    ###################################################
    # Input Data
    # This part assumes that you've created the data using the calibration/examples/1_create_sample_DSTs.sh
    input_files_test = [os.path.join(os.path.abspath(data_dir), '*.root')]

    ###################################################
    # Test Calibration Setup

    # Make a bunch of test calibrations
    col_test = register_module('CaTest')
    # Specific parameter to our test collector, proportional to the probability of algorithm requesting iteration.
    col_test.param('spread', 15)

    alg_test = TestCalibrationAlgorithm()

    cal_test = Calibration(name='TestCalibration',
                           collector=col_test,
                           algorithms=alg_test,
                           input_files=input_files_test)

    cal_test.max_files_per_collector_job = 1
    cal_test.max_iterations = 5  # Each calibration will end iteration after this many attempts (if reached) starting from 0.

    # We don't want to run collector jobs on these runs (if possible) and we don't want the algorithm to use data from
    # these runs. However, the algorithm strategy may also merge IoVs across the gaps left by the ignored runs.
    # What the strategy does with missing runs depends on the AlgorithmStrategy and any configuration you have made.
    cal_test.ignored_runs = [ExpRun(0, 2), ExpRun(0, 3), ExpRun(0, 4)]

    ###################################################
    # Create a CAF instance to configure how we will run
    cal_fw = CAF()

    # You could alternatively set the same ignored_runs for every Calibration by setting it here.
    # Note that setting cal_test.ignored_runs will override the value set from here.

    # cal_fw = CAF(calibration_defaults={'ignored_runs':[ExpRun(0,2)])

    # Add in our list of calibrations
    cal_fw.add_calibration(cal_test)
    cal_fw.backend = backends.Local(max_processes=4)
    # Start her up!
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
