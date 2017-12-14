# This steering file shows pretty much the most minimal setup for
# running the CAF. You will need to have data already from running
# calibration/examples/1_create_sample_DSTs.sh or just make your own
# and change the input data below.

from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF
use_local_database("localDB/database.txt", "localDB")


def main():

    main = create_path()
    main.add_module('EventInfoSetter',
                    expList=[0],
                    evtNumList=[1],
                    runList=[1630])
    main.add_module('Gearbox')
    main.add_module('Geometry', components=['CDC'])
    process(main, 1)

    data_dir = '/gpfs/fs02/belle2/users/dvthanh/201702_unpacked/'

    ###################################################
    # Input Data
    # This part assumes that you've created the data using the calibration/examples/1_create_sample_DSTs.sh
    # We'll use the same data for all calibrations but this is not a requirement in general.
    # ALWAYS USE ABSOLUTE PATHS TO THE FILES! i.e. remember to os.path.abspath(file) them
    input_files_test = []
    runs = []
    with open('runlist') as runlist:
        lines = runlist.readlines()
        for line in lines:
            runs.append('cr.' + line.rstrip() + '.root')
    input_files_test = [data_dir + f for f in runs]

    ###################################################
    # Test Calibration Setup
    algo = Belle2.CDC.CrudeT0CalibrationAlgorithm()  # Getting a calibration algorithm instance

    # Create a single calibration from a collector module name + algorithm + input files
    cal_test = Calibration(
        name="CrudeT0Calibration",
        collector="CDCCrudeT0Collector",
        algorithms=algo,
        input_files=input_files_test)

    ###################################################
    # Create a CAF instance and add the calibration to it. Should run on one CPU core locally by default.
    cal_fw = CAF()
    cal_fw.add_calibration(cal_test)
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main()
