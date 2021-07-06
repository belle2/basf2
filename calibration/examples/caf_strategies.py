##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This steering file shows pretty much the most minimal setup for
# running the CAF. You will need to have data already from running
# calibration/examples/1_create_sample_DSTs.sh or just make your own
# and change the input data below.

import basf2 as b2

import os
import sys

from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF

b2.set_log_level(b2.LogLevel.INFO)
# add time stamp to all INFO messages
# currentInfo = logging.get_info(LogLevel.INFO)
# logging.set_info(LogLevel.INFO, currentInfo | LogInfo.TIMESTAMP)


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
    from caf.strategies import SingleIOV, SequentialRunByRun, SimpleRunByRun, SequentialBoundaries  # noqa
    # The default value is SingleIOV, you don't have to set this, it is done automatically.
    # SingleIOV just takes all of the runs as one big IoV and executes the algorithm once on all of their data.
    # You can use granularity='run' or granularity='all' for the collector when using this strategy.

    # cal_test.strategies = SingleIOV

    # The SequentialRunByRun strategy executes your algorithm over runs
    # individually to give you payloads for each one (if successful)
    # If there wasn't enough data in a run to give a success, it tries to merge with the next run's data
    # and re-execute.
    # You should only use granularity='run' for the collector when using this strategy.

    cal_test.strategies = SequentialRunByRun

    # The SimpleRunByRun strategy executes your algorithm over runs
    # individually to give you payloads for each one (if successful)
    # It will not do any merging of runs which didn't contain enough data.
    # So failure is expected if your algorithm requires a large amount of data compared to run length.
    # You should only use granularity='run' for the collector when using this strategy.

    # cal_test.strategies = SimpleRunByRun

    # The SequentialBoundaries strategy executes your algorithm over all
    # the runs in a time period contained between two boundaries to give
    # you payloads for each one time period (if successful)
    # If there wasn't enough data in a run to give a success, it tries to
    # merge with the next time period's data and re-execute.
    # You should only use granularity='run' for the collector when using this strategy.
    # The run Boundaries can be either passed directy in the parameter dictionary
    # of the algorithm (key `payload_boundaries`) or calculated by the algorithm itself
    # in the function `isBoundaryRequired`.

    # cal_test.strategies = SequentialBoundaries
    # cal_test.algorithms[0].params = {"payload_boundaries" : [(0,1), (0,4), (0,7)]}

    ###################################################
    # Create a CAF instance and add the calibration to it. Should run on one CPU core locally by default.
    cal_fw = CAF()
    cal_fw.add_calibration(cal_test)
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
