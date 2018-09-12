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


def main(argv):
    if len(argv) == 1:
        data_dir = argv[0]
    else:
        print("Usage: python3 caf_multiple_options.py <data directory>")
        sys.exit(1)

    ###################################################
    # Input Data
    # This part assumes that you've created the data using the calibration/examples/1_create_sample_DSTs.sh
    # We'll use the same data for all calibrations but this is not a requirement in general.
    input_files_test = [os.path.join(os.path.abspath(data_dir), '*.root')]

    ###################################################
    # Test Calibration Setup

    # Define a function to be performed before the algorithm.execute(iov)
    def pre_alg_test(algorithm, iteration):
        """
        Just to show that the function is correctly applied
        """
        B2INFO("Running Test Algorithm Setup For Iteration {0}".format(iteration))
        B2INFO("Can access the {0} class from Calibration().pre_algorithms.".format(algorithm.__cppname__))

    # Make a bunch of test calibrations
    calibrations = []
    for i in range(1, 5):
        col_test = register_module('CaTest')
        col_test.set_name('Test{}'.format(i))   # Sets the prefix of the collected data in the datastore.
        # Allows us to execute algorithm over all input data, in one big IoV.
        col_test.param('granularity', 'all')
        # Specific parameter to our test collector, proportional to the probability of algorithm requesting iteration.
        col_test.param('spread', 15)

        alg_test = TestCalibrationAlgorithm()
        # Since we're using several instances of the same test algorithm here, we still want the database entries to have
        # different names. TestCalibrationAlgorithm outputs to the database using the prefix name so we change it
        # slightly for each calibration. Not something you'd usually have to do.
        alg_test.setPrefix('Test{}'.format(i))  # Must be the same as colllector prefix

        cal_test = Calibration(name='TestCalibration{}'.format(i),
                               collector=col_test,
                               algorithms=alg_test,
                               input_files=input_files_test)
        cal_test.pre_algorithms = pre_alg_test

        cal_test.max_files_per_collector_job = 1
        # If you have some local databases or want to override the default global tag for this calibration you can do that
        # with these functions
#        cal_test.use_local_database("mylocaldb/database.txt")
#        cal_test.use_central_database("BelleII_GlobalTag_Tutorial")
        calibrations.append(cal_test)

    # Define dependencies. In this case the classic diamond e.g.
    #    2
    #   / \
    #  1   4
    #   \ /
    #    3
    calibrations[1].depends_on(calibrations[0])
    calibrations[2].depends_on(calibrations[0])
    calibrations[3].depends_on(calibrations[2])
    calibrations[3].depends_on(calibrations[1])

    ###################################################
    # Create a CAF instance to configure how we will run
    cal_fw = CAF()
    cal_fw.max_iterations = 5  # Each calibration will end iteration after this many attempts (if reached)
    # Add in our list of calibrations
    for cal in calibrations:
        cal_fw.add_calibration(cal)
    # Subjobs from collector jobs being split over input files can be paralellized.
    # Also Calibrations 1 and 2, can be run at the same time.
    # If you have 4 cores this backend will run them whenever one of the 4 processes becomes available
    # For larger data or more calibrations, consider using the LSF or PBS batch system backends at your site
    cal_fw.backend = backends.Local(max_processes=4)
    cal_fw.output_dir = 'cal_test_results'  # Can change where your calibration runs
    # Start her up!
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
