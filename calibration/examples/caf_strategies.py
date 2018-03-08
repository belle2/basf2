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
    alg_test.setMinEntries(15000)  # This algorithm provides a setting to change when c_NotEnoughData is returned

    # Create a single calibration from a collector module name + algorithm + input files
    cal_test = Calibration(name="TestCalibration", collector="CaTest", algorithms=alg_test, input_files=input_files_test)

    # Here we set the AlgorithmStrategy for our algorithm
    from caf.strategies import SequentialRunByRun, SingleIOV
    # The SequentialRunByRun strategy executes your algorithm over runs
    # individually to give you payloads for each one (if successful)
    cal_test.strategies = SequentialRunByRun
    # The default value is SingleIOV, you don't have to set this, it is done automatically.
    # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on them all.
    # cal_test.strategies = SingleIOV

    ###################################################
    # Create a CAF instance and add the calibration to it. Should run on one CPU core locally by default.
    cal_fw = CAF()
    cal_fw.add_calibration(cal_test)
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
