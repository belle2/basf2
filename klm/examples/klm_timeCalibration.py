#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
set_log_level(LogLevel.INFO)
# add time stamp to all INFO messages
# currentInfo = logging.get_info(LogLevel.INFO)
# logging.set_info(LogLevel.INFO, currentInfo | LogInfo.TIMESTAMP)

import os
import sys

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf.backends import LSF
from caf.utils import IoV
from caf.utils import CentralDatabase
from caf.utils import ExpRun


def main(argv):

    if len(argv) == 2:
        expN = argv[0]
        output_dir = argv[1]
    else:
        sys.exit('Usage: basf2 klm_tc.py <expNum[07,08]> <output directory>')

    set_debug_level(20)
    print(argv)
    # =====================================================
    # Input Data
    # ALWAYS USE ABSOLUTE PATHS TO THE FILES!
    # i.e. remember to os.path.abspath(file) them
    input_files = []

    # Exp07 input (proc10)
    if expN == '07':
        data_dir = '/group/belle2/dataprod/Data/OfficialReco/proc10/e0007/4S/GoodRuns/r0??01/skim/hlt_mumu_2trk/cdst/sub00/'
        input_files.append(os.path.join(os.path.abspath(data_dir), 'cdst.physics.*.root'))

    # Exp08 input (proc10)
    if expN == '08':
        data_dir = '/group/belle2/dataprod/Data/OfficialReco/proc10/e0008/4S/GoodRuns/r000??/skim/hlt_mumu_2trk/cdst/sub00/'
        input_files.append(os.path.join(os.path.abspath(data_dir), 'cdst.physics.*.root'))
        data_dir = '/group/belle2/dataprod/Data/OfficialReco/proc10/e0008/Continuum/GoodRuns/r000??/skim/hlt_mumu_2trk/cdst/sub00/'
        input_files.append(os.path.join(os.path.abspath(data_dir), 'cdst.physics.*.root'))
        data_dir = '/group/belle2/dataprod/Data/OfficialReco/proc10/e0008/Scan/GoodRuns/r000??/skim/hlt_mumu_2trk/cdst/sub00/'
        input_files.append(os.path.join(os.path.abspath(data_dir), 'cdst.physics.*.root'))

    print('Add pre_collector_path')
    gearbox = register_module('Gearbox')
    geobuilder = register_module('Geometry')
    pre_path = create_path()
    pre_path.add_module(gearbox)
    pre_path.add_module(geobuilder)
    # pre_path.add_module('Progress')

    # =====================================================
    # Calibration ingredients setup
    coll = register_module('KLMTimeCalibrationCollector')

    algo = Belle2.KLMTimeCalibrationAlgorithm()
    # algo.useFit()
    # algo.isMC()
    algo.useEvtT0()
    algo.setLowerLimit(100)

    cal = Calibration('KLMTimeCalibration', coll, algo, input_files)
    cal.reset_database()
    # cal.use_central_database('master_2019-04-10')  # Used for MC test
    cal.use_central_database('data_reprocessing_proc10')  # New calibrations
    cal.use_central_database('online_proc10')
    cal.pre_collector_path = pre_path
    # =====================================================
    # Setup file number in each of the subjob
    cal.max_files_per_collector_job = 5
    # =====================================================
    # Setup the queue kind
    cal.backend_args = {'queue': 's'}
    cal.heartbeat = 30
    cal.collector_full_update_interval = 300

    # =====================================================
    # Creat a CAF instance and add calibreation.
    cal_fw = CAF()

    cal_fw.add_calibration(cal)

    # =====================================================
    # Set job backend
    cal_fw.backend = LSF()

    # =====================================================
    # Set outout path
    cal_fw.output_dir = output_dir

    # =====================================================
    # Set subset used fon calibration
    # iov_to_calibrate = IoV(exp_low=3, run_low=-1, exp_high=3, run_high=-1)
    print('debug output before run')

    # cal_fw.run(iov=iov_to_calibrate)
    cal_fw.run()
    print("End of CAF processing.")


if __name__ == "__main__":
    main(sys.argv[1:])
