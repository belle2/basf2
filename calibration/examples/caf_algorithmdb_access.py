##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This steering file shows off algorithms accessing database constants
# from within the CAF. There's actually no special changes to the CAF script,
# we're just going to use algorithms that do this accessing.
#
# You should check the stdout of the algorithm output to see the database constants
# being used.
#
# We're going to pass the database constants from one Calibration to another
# that depends on it. AND we're going to show that when you have multiple algorithms
# in a SINGLE Calibration, the constants are passed forwards to the next as they get saved.

import basf2 as b2

import os
import sys

from ROOT.Belle2 import TestDBAccessAlgorithm

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
    input_files = [os.path.join(os.path.abspath(data_dir), '*.root')]

    ###################################################
    # Test Calibration Setup
    dep_col = b2.register_module('CaTest')
    dep_col.set_name('Dependent')   # Sets the prefix of the collected data in the datastore.
    # Allows us to execute algorithm over input data in a run-by-run way so we can see DB constants from each run.
    dep_col.param('granularity', 'run')
    # Specific parameter to our test collector, proportional to the probability of algorithm requesting iteration.
    dep_col.param('spread', 15)

    dep_alg = TestDBAccessAlgorithm()
    # Since we're using several instances of the same test algorithm here, we still want the database entries to have
    # different names. TestCalibrationAlgorithm outputs to the database using the prefix name so we change it
    # for each calibration. Not something you'd usually have to do. And you could instead use whatever name you want,
    # we just use the prefix as a handy string.
    dep_alg.setPrefix('Dependent')  # Must be the same as colllector prefix

    # Create our Calibration object
    dep_cal = Calibration(name='Dependent_Calibration',
                          collector=dep_col,
                          algorithms=dep_alg,
                          input_files=input_files)

    # Let's split up the collector jobs by files
    dep_cal.max_files_per_collector_job = 1
    dep_cal.max_iterations = 5

    # This one runs last and picks up the final DB constants from the first one
    last_col = b2.register_module('CaTest')
    last_col.set_name('Last')   # Sets the prefix of the collected data in the datastore.
    # Allows us to execute algorithm over input data in a run-by-run way so we can see DB constants from each run.
    last_col.param('granularity', 'run')
    # Specific parameter to our test collector, proportional to the probability of algorithm requesting iteration.
    last_col.param('spread', 15)

    last_alg = TestDBAccessAlgorithm()
    last_alg.setGeneratePayloads(False)
    last_alg.setPrefix('Last')  # Must be the same as colllector prefix

    # Create our Calibration object
    last_cal = Calibration(name='Last_Calibration',
                           collector=last_col,
                           algorithms=last_alg,
                           input_files=input_files)

    # Let's split up the collector jobs by files
    last_cal.max_files_per_collector_job = 1
    last_cal.max_iterations = 5

    # Define dependencies
    last_cal.depends_on(dep_cal)

    ###################################################
    # Create a CAF instance to configure how we will run
    cal_fw = CAF()
    cal_fw.add_calibration(dep_cal)
    cal_fw.add_calibration(last_cal)
    # Subjobs from collector jobs being split over input files can be paralellized.
    # Also Calibrations 1 and 2, can be run at the same time.
    # If you have 4 cores this backend will run them whenever one of the 4 processes becomes available
    # For larger data or more calibrations, consider using the LSF or PBS batch system backends at your site
    cal_fw.backend = backends.Local(max_processes=4)
    # Start her up!
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
