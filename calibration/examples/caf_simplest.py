# This steering file shows pretty much the most minimal setup for
# running the CAF. You will need to have data already from running
# calibration/examples/1_create_sample_DSTs.sh or just make your own
# and change the input data below.

from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys

import ROOT
from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF


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
    alg_test = TestCalibrationAlgorithm()  # Getting a calibration algorithm instance

    # Create a single calibration from a collector module name + algorithm + input files
    cal_test = Calibration(name="TestCalibration", collector="CaTest", algorithms=alg_test, input_files=input_files_test)
    # The RootOutput.root file will be automatically picked up as output of the collector.
    # But CaTest also produces .mille files so let's make sure we grab them too (and might as well get the .xml)
    cal_test.output_patterns.append('Belle2FileCatalog.xml')
    cal_test.output_patterns.append('*.mille')

    ###################################################
    # Create a CAF instance and add the calibration to it. Should run on one CPU core locally by default.
    cal_fw = CAF()
    cal_fw.add_calibration(cal_test)
    cal_fw.run()
    print("End of CAF processing.")

if __name__ == "__main__":
    main(sys.argv[1:])
