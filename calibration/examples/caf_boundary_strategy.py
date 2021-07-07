##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# You will need to have data already from running
# calibration/examples/1_create_sample_DSTs.sh or just make your own
# and change the input data below.

import basf2 as b2

import sys
from pathlib import Path

from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf.utils import IoV
from caf.strategies import SequentialBoundaries


b2.set_log_level(b2.LogLevel.DEBUG)


def main(argv):
    if len(argv) == 1:
        data_dir = argv[0]
    else:
        print("Usage: basf2 caf_boundary_strategy.py <data directory>")
        sys.exit(1)

    ###################################################
    # Input Data
    # This part assumes that you've created the data using the calibration/examples/1_create_sample_DSTs.sh
    # We'll use the same data for all calibrations but this is not a requirement in general.
    input_files_test = []
    input_files_test.append(Path(Path(data_dir).absolute(), '*.root').as_posix())

    ###################################################
    # Test Calibration Setup
    alg_test = TestCalibrationAlgorithm()  # Getting a calibration algorithm instance
    alg_test.setMinEntries(15000)  # This algorithm provides a setting to change when c_NotEnoughData is returned
    alg_test.setAllowedMeanShift(0.1)  # This alters how often boundaries will be requested (smaller = more often)

    # Create a single calibration from a collector module name + algorithm + input files
    cal_test = Calibration(name="TestCalibration", collector="CaTest", algorithms=alg_test, input_files=input_files_test)

    # The SequentialBoundaries strategy executes your algorithm over runs but only where you have defined a boundary
    # for new payloads. In order to run this strategy your algorithm must define a "isBoundaryRequired" member function.
    #
    # You should only use granularity='run' for the collector when using this strategy.

    cal_test.strategies = SequentialBoundaries

    cal_test.algorithms[0].params["iov_coverage"] = IoV(0, 0, -1, -1)

    ###################################################
    # Create a CAF instance and add the calibration to it. Should run on one CPU core locally by default.
    cal_fw = CAF()
    cal_fw.add_calibration(cal_test)
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
