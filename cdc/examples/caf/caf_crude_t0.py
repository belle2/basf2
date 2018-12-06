from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf import backends

data_dir = '/gpfs/fs02/belle2/users/dvthanh/201702_unpacked/'
runs = []
with open('runlist') as runlist:
    lines = runlist.readlines()
    for line in lines:
        runs.append('cr.' + line.rstrip() + '.root')
input_files_test = [data_dir + f for f in runs]


def main():

    def cdc_pre_algorithm(algorithm, iteration):
        B2INFO("Running pre_algorithm function")
        #        evtinfo = register_module('EventInfoSetter', evtNumList=[1], runList=[1630], expList=[1])
        #        gear = register_module('Gearbox')
        #        geom = register_module('Geometry', components=['CDC'])
        #        evtinfo.initialize()
        #        gear.initialize()
        #        geom.initialize()

    calibrations = []
    for i in range(1):
        col_test = register_module('CDCCrudeT0Collector')
        col_test.set_name('CDCCrudeT0{}'.format(i))  # Sets the prefix of the collected data in the datastore
        col_test.param('granularity', 'all')  # Allows us to execute algorithm over all data, in one big IoV

        alg_test = Belle2.CDC.CrudeT0CalibrationAlgorithm()
        # Since we're using several instances of the same test algorithm here, we still want the database entries to have
        # different names. TestCalibrationAlgorithm outputs to the database using the prefix name so we change it
        # slightly for each calibration. Not something you'd usually have to do.
        alg_test.setPrefix('CDCCrudeT0{}'.format(i))  # Must be the same as colllector prefix

        cal_test = Calibration(name='CrudeT0Calibration{}'.format(i),
                               collector=col_test,
                               algorithms=alg_test,
                               input_files=input_files_test)
        cal_test.pre_algorithms = cdc_pre_algorithm
        cal_test.max_files_per_collector_job = 1
        cal_test.backend_args = {"queue": "s"}
        #        cal_test.use_central_database('')
        cal_test.use_local_database('database.txt', '/home/belle/muchida/basf2/release/work/caf/test/localDB')
        calibrations.append(cal_test)

    ###################################################
    # Create a CAF instance
    cal_fw = CAF()
    # Add in our list of calibrations
    for cal in calibrations:
        cal_fw.add_calibration(cal)
    # Use the default LSF backend setup, can view the default options in calibration/data/backends.cfg
    cal_fw.backend = backends.LSF()
    # Since we're using the LSF batch system we'll up the heartbeat from the default to query for when the jobs are all finished
    # No point spamming it
    cal_fw.heartbeat = 15
    # Start running
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main()
