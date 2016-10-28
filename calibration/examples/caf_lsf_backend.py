# This steering file shows pretty much the most minimal setup for
# running the CAF using the LSF (qsub) batch system backend with multiple test calibrations.
# You will need to have data already from running calibration/examples/1_create_sample_DSTs.sh
# or just make your own and change the input data below.

from basf2 import *
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
        print("Usage: basf2 CAF_simplest.py <data directory>")
        sys.exit(1)

    ###################################################
    # Input Data
    # This part assumes that you've created the data using the calibration/examples/1_create_sample_DSTs.sh
    # We'll use the same data for all calibrations but this is not a requirement in general.
    input_files_test = []
    for run in range(1, 5):
        cosmics_file_path = os.path.join(data_dir, 'DST_exp1_run{0}.root'.format(run))
        input_files_test.append(cosmics_file_path)

    ###################################################
    # Test Calibration Setup
    # Make a bunch of test calibrations
    calibrations = []
    for i in range(1, 3):
        col_test = register_module('CaTest')
        col_test.set_name('Test{}'.format(i))  # Sets the prefix of the collected data in the datastore
        col_test.param('spread', 15)  # Proportional to the probability of algorithm requesting iteration
        col_test.param('granularity', 'all')  # Allows us to execute algorithm over all data, in one big IoV

        alg_test = TestCalibrationAlgorithm()
        # Since we're using several instances of the same test algorithm here, we still want the database entries to have
        # different names. TestCalibrationAlgorithm outputs to the database using the prefix name so we change it
        # slightly for each calibration. Not something you'd usually have to do.
        alg_test.setPrefix('Test{}'.format(i))  # Must be the same as colllector prefix

        cal_test = Calibration(name='TestCalibration{}'.format(i),
                               collector=col_test,
                               algorithms=alg_test,
                               input_files=input_files_test)
        cal_test.output_patterns.append('Belle2FileCatalog.xml')
        cal_test.output_patterns.append('*.mille')

        calibrations.append(cal_test)

    ###################################################
    # Create a CAF instance
    cal_fw = CAF()
    cal_fw.heartbeat = 15
    # Add in our list of calibrations
    for cal in calibrations:
        cal_fw.add_calibration(cal)

    # Use the default LSF backend setup, can view the default options in calibration/data/caf.cfg
    cal_fw.backend = backends.LSF()

    # You could specify a local/different release e.g. the head
#    cal_fw.backend.release = "/data/ddossett/software/release"
    # Or specify a different queue to submit to
#    cal_fw.backend.queue = "long"
    # Or specify a config file to load all options at once (Uses Python ConfigParser)
#    cal_fw.backend.load_from_config("file/path/to/config.cfg", "YourLSFSetupSection")

    # Can override the basf2 setup for the batch jobs directly if you have something weird
#    cal_fw.backend.basf2_setup = [export "VO_BELLE2_SW_DIR=/cvmfs/belle.cern.ch/sl6\n",
#                            "SETUPBELLE2_CVMFS=/cvmfs/belle.cern.ch/tools.new/setup_belle2\n",
#                            "CAF_RELEASE_LOCATION=/cvmfs/belle.cern.ch/sl6/releases/build-2016-09-09\n",
#                            "source $SETUPBELLE2_CVMFS\n",
#                            "pushd $CAF_RELEASE_LOCATION > /dev/null\n",
#                            "setuprel\n",
#                            "popd > /dev/null\n"]

    # Start running
    cal_fw.run()
    print("End of CAF processing.")

if __name__ == "__main__":
    main(sys.argv[1:])
