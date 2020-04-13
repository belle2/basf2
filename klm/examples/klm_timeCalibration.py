#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys

import basf2 as b2
import modularAnalysis as ma

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import TestCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf.backends import LSF
from caf.utils import IoV
from caf.utils import CentralDatabase
from caf.utils import ExpRun


b2.set_log_level(b2.LogLevel.INFO)


def main(argv):

    if len(argv) == 2:
        expN = argv[0]
        output_dir = argv[1]
    else:
        sys.exit('Usage: basf2 klm_tc.py <expNum[07,08]> <output directory>')

    b2.set_debug_level(20)
    print(argv)
    # =====================================================
    # Input Data
    # ALWAYS USE ABSOLUTE PATHS TO THE FILES!
    # i.e. remember to os.path.abspath(file) them
    input_files = []

    # Exp07 input (proc10)
    if expN == '07':
        data_dir = '/group/belle2/dataprod/Data/OfficialReco/proc10/e0007/4S/GoodRuns/r03322/skim/hlt_mumu_2trk/cdst/sub00/'
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
    gearbox = b2.register_module('Gearbox')
    geobuilder = b2.register_module('Geometry')
    pre_path = b2.create_path()
    pre_path.add_module(gearbox)
    pre_path.add_module(geobuilder)
    # pre_path.add_module('Progress')
    print('Add Gearbox and Geometry to pre_collector_path. Done.')

    # =====================================================
    # Calibration ingredients setup
    ma.fillParticleList('mu+:cali', '1 < p and p < 11', path=pre_path)
    coll = b2.register_module('KLMTimeCalibrationCollector')

    algo = Belle2.KLMTimeCalibrationAlgorithm()
    # algo.useFit()
    # algo.isMC()
    algo.useEvtT0()
    algo.setLowerLimit(100)

    cal = Calibration('KLMTimeCalibration', coll, algo, input_files)
    cal.reset_database()
    # With use_central_database, the last GT has highest priority
    # cal.use_central_database('master_2019-04-10')  # Used for MC test
    cal.use_central_database('data_reprocessing_proc10')  # New calibrations
    # cal.use_central_database('online_proc10')
    # cal.use_central_database('data_reprocessing_prompt_rel4_patchb')
    cal.use_central_database('klm_alignment_testing')  # To avoid errors from alignment
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
