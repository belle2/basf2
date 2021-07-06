##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This steering file shows off how you can ignore runs from the input data of a
# Calibration. And how the SequentialRunByRun strategy deals with gaps in the input data

import basf2 as b2

import os
import sys

from ROOT.Belle2 import TestCalibrationAlgorithm

from caf.framework import Calibration, CAF
from caf.utils import ExpRun, IoV
from caf.strategies import SequentialRunByRun
from caf.backends import Local

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
    # We want to see what happens when c_NotEnoughData is returned
    alg_test.setMinEntries(10000)

    cal_test = Calibration(name='TestCalibration',
                           collector=col_test,
                           algorithms=alg_test,
                           input_files=input_files_test)

    # We don't want to run collector jobs on these runs (if possible) and we don't want the algorithm to use data from
    # these runs. However, the algorithm strategy may also merge IoVs across the gaps left by the ignored runs.
    # What the strategy does with missing runs depends on the AlgorithmStrategy and any configuration you have made.
    cal_test.ignored_runs = [ExpRun(0, 2), ExpRun(0, 3)]

    # We use a different algorithm strategy for every algorithm (only one) in this Calibration
    cal_test.strategies = SequentialRunByRun

    # This parameter is used by SequentialRunByRun to define a total IoV that the combination of all IoVs defines.
    # It must be strictly larger than the IoV of all input data, OR the IoV in cal_fw.run(iov=...) if defined.
    # This allows us to define a larger total IoV so that the first payload IoV MUST START at the ExpRun(exp_low, run_low)
    # and the final IoV MUST END at ExpRun(exp_high, run_high) e.g.
    #
    # Here we define iov_coverage = IoV(0, 1, 0, 15). This means that the first payload created will be of the form
    # IoV(0, 1, ?, ?) where we don't yet know what the highest (exp,run) will be as it depends on the data.
    # The last payload we create must have the form IoV(?, ?, 0, 15) but we can't yet know the lowest (exp,run)
    #
    # It is entirely possible that in the final database there will be only one payload of IoV(0, 1, 0, 15), or
    # several such as [IoV(0, 1, 0, 5), IoV(0, 6, 0, 10), IoV(0, 11, 0, 15)]
    cal_test.algorithms[0].params["iov_coverage"] = IoV(0, 1, 0, 15)

    # This parameter defines the step size of the SequentialRunByRun strategy i.e. how many runs of data we add
    # each time we execute.
    # So initially an execution of the algorithm will use this many runs-worth of data (if possible).
    # If c_NotEnoughData is returned on this execution, we will add the next 'step_size' number of runs and execute again.
    # Then repeat until we get c_Iterate or c_OK.
    #
    # The default value is 1.
    cal_test.algorithms[0].params["step_size"] = 1

    cal_test.max_files_per_collector_job = 1

    ###################################################
    # Create a CAF instance to configure how we will run
    cal_fw = CAF()

    # You could alternatively set the same ignored_runs for every Calibration by setting it here.
    # Note that setting cal_test.ignored_runs will override the value set from here.

    # cal_fw = CAF(calibration_defaults={'ignored_runs':[ExpRun(0,2), ExpRun(0, 3)])

    cal_fw.add_calibration(cal_test)
    cal_fw.backend = Local(max_processes=4)

    # The iov value here allows you to set an IoV that all your input files/executed runs must overlap.
    # Any input files not overlapping this IoV will be ignored.
    # HOWEVER, if you have an input file containing multiple runs the file IoV may overlap this IoV. But it may still
    # contain runs outside of it.
    # In this case the CAF will still use the file in collector jobs, BUT any runs collected will not be used in the
    # algorithm step if they exist outside of this IoV.
    #
    # To explicitly prevent certain runs from within this IoV being used, you should use the ignored_runs
    # attribute of the Calibration. Or make sure that the input data files do not contain data from those runs at all.
    cal_fw.run(iov=IoV(0, 3, 0, 11))
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
