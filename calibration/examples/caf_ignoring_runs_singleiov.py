##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This steering file shows off some more options that can be configured
# and how to run multiple dependent calibrations. You will need to have
# data already from running calibration/examples/1_create_sample_DSTs.sh
# or just make your own any change the input data below.

import basf2 as b2

import os
import sys

from ROOT.Belle2 import TestCalibrationAlgorithm

from caf.framework import Calibration, CAF
from caf.utils import ExpRun, IoV

b2.set_log_level(b2.LogLevel.INFO)


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
    col_test = b2.register_module('CaTest')
    # Specific parameter to our test collector, proportional to the probability of algorithm requesting iteration.
    col_test.param('spread', 15)

    alg_test = TestCalibrationAlgorithm()

    cal_test = Calibration(name='TestCalibration',
                           collector=col_test,
                           algorithms=alg_test,
                           input_files=input_files_test)

    # We don't want to run collector jobs on these runs (if possible) and we don't want the algorithm to use data from
    # these runs. However, the algorithm strategy may also merge IoVs across the gaps left by the ignored runs.
    # What the strategy does with missing runs depends on the AlgorithmStrategy and any configuration you have made.
    cal_test.ignored_runs = [ExpRun(0, 2), ExpRun(0, 3), ExpRun(0, 5), ExpRun(0, 6)]

    # The framework.Algorithm class (not the CalibrationAlgorithm) has a params attribute.
    # For the SingleIoV strategy (default), setting the "apply_iov" to an IoV object will cause the final payload
    # to have this IoV.
    # This happens REGARDLESS of whether data from this IoV was used in this calibration.
    # Think of it as an optional override for the output IoV.
    # This is ONLY true for the SingleIoV strategy.
    #
    # If you don't set this, then the output IoV will come from the IoV of all executed runs.
    # In this case it would be IoV(0,1,0,1) as we are excluding all other runs manually either by the ignored_runs,
    # or by the cal_fw.run(iov=)
    cal_test.algorithms[0].params["apply_iov"] = IoV(0, 1, 0, 10)

    ###################################################
    # Create a CAF instance to configure how we will run
    cal_fw = CAF()

    # You could alternatively set the same ignored_runs for every Calibration by setting it here.
    # Note that setting cal_test.ignored_runs will override the value set from here.

    # cal_fw = CAF(calibration_defaults={'ignored_runs':[ExpRun(0,2), ExpRun(0, 3)])

    cal_fw.add_calibration(cal_test)

    # The iov value here allows you to set an IoV that all your input files/executed runs must overlap.
    # Any input files not overlapping this IoV will be ignored.
    # HOWEVER, if you have an input file containing multiple runs the file IoV may overlap this IoV. But it may still
    # contain runs outside of it.
    # In this case the CAF will still use the file in collector jobs, BUT any runs collected will not be used in the
    # algorithm step if they exist outside of this IoV.
    #
    # To explicitly prevent certain runs from within this IoV being used, you should use the ignored_runs
    # attribute of the Calibration. Or make sure that the input data files do not contain data from those runs at all.
    cal_fw.run(iov=IoV(0, 3, 0, 9))
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
