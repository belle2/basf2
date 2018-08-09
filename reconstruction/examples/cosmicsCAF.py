#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# Run the dE/dx calibration with the CAF:
#  - collect the samples with the collector modules
#  - run the calibrations with the given algorithms
#  - produce output database files
#
# Usage: basf2 calibrateCosmics.py
#
# Input: cosmics data
# Output: calibration_results
#
# Contributors: Jake Bennett
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
import os
import sys
import ROOT
from ROOT.Belle2 import CDCDedxMomentumAlgorithm, CDCDedxCosineAlgorithm
from ROOT.Belle2 import CDCDedxWireGainAlgorithm, CDCDedxRunGainAlgorithm
from ROOT.Belle2 import CDCDedx2DCorrectionAlgorithm, CDCDedx1DCleanupAlgorithm
from caf.framework import Calibration, CAF

ROOT.gROOT.SetBatch(True)

# Specify the input file(s)
input_files = [os.path.abspath(
    '/group/belle2/users/jbennett/GCR2/release-01-02-01/DB00000359/r00282/dst.cosmic.0002.0282.f00000.root')]

# Modify the collector to apply other calibration constants
momentum_col = register_module('CDCDedxElectronCollector')
momentum_col_params = {'cleanupCuts': False,
                       'scaleCor': False,
                       'momentumCor': False,
                       'momentumCorFromDB': False,
                       'cosineCor': False,
                       }
momentum_col.param(momentum_col_params)

mod_col = register_module('CDCDedxElectronCollector')
mod_col_params = {'cleanupCuts': True,
                  'scaleCor': False,
                  'momentumCor': False,
                  'momentumCorFromDB': False,
                  'cosineCor': False,
                  }
mod_col.param(mod_col_params)

# Define the calibration algorithms
momentum_alg = CDCDedxMomentumAlgorithm()
cosine_alg = CDCDedxCosineAlgorithm()
run_gain_alg = CDCDedxRunGainAlgorithm()
wire_gain_alg = CDCDedxWireGainAlgorithm()
twod_alg = CDCDedx2DCorrectionAlgorithm()
oned_alg = CDCDedx1DCleanupAlgorithm()

# Create Calibration objects from Collector, Algorithm(s), and input files
momentum_cal = Calibration(
    name='MomentumCalibration',
    collector=momentum_col,
    algorithms=[momentum_alg],
    input_files=input_files)

cosine_cal = Calibration(
    name='CosineCalibration',
    collector=mod_col,
    algorithms=[cosine_alg],
    input_files=input_files)

wire_gain_cal = Calibration(
    name='WireGainCalibration',
    collector=mod_col,
    algorithms=[wire_gain_alg],
    input_files=input_files)

run_gain_cal = Calibration(
    name='RunGainCalibration',
    collector=mod_col,
    algorithms=[run_gain_alg],
    input_files=input_files)

other_cal = Calibration(
    name='OtherDedxCalibrations',
    collector=mod_col,
    algorithms=[twod_alg, oned_alg],
    input_files=input_files)

# Use the constants in the local databases
momdb = 'calibration_results/MomentumCalibration/outputdb'
cosdb = 'calibration_results/CosineCalibration/outputdb'
wgdb = 'calibration_results/WireGainCalibration/outputdb'
rgdb = 'calibration_results/RunGainCalibration/outputdb'
localdb = '/home/belle2/jbennett/CRDATA/release-00-09-01/DB00000266/r03118/calib/localdb/database.txt'

momentum_cal.use_local_database(localdb)

cosine_cal.use_local_database(localdb)
cosine_cal.use_local_database(momdb)

wire_gain_cal.use_local_database(localdb)
wire_gain_cal.use_local_database(momdb)
wire_gain_cal.use_local_database(cosdb)

run_gain_cal.use_local_database(localdb)
run_gain_cal.use_local_database(momdb)
run_gain_cal.use_local_database(cosdb)
run_gain_cal.use_local_database(wgdb)

other_cal.use_local_database(localdb)
other_cal.use_local_database(momdb)
other_cal.use_local_database(cosdb)
other_cal.use_local_database(wgdb)
other_cal.use_local_database(rgdb)

# Add a pre-collector path to apply old calibration constants
correct_for_mom = create_path()
correct_for_mom.add_module('CDCDedxCorrection', scaleCor=False, momentumCor=False,
                           momentumCorFromDB=False, cosineCor=True,
                           runGain=True, wireGain=True)
momentum_cal.pre_collector_path = correct_for_mom

correct_for_cos = create_path()
correct_for_cos.add_module('CDCDedxCorrection', scaleCor=False, momentumCor=True,
                           momentumCorFromDB=True, cosineCor=False,
                           runGain=True, wireGain=True)
cosine_cal.pre_collector_path = correct_for_cos
cosine_cal.depends_on(momentum_cal)

correct_for_wire_gain = create_path()
correct_for_wire_gain.add_module('CDCDedxCorrection', scaleCor=False, momentumCor=True,
                                 momentumCorFromDB=True, cosineCor=True,
                                 runGain=True, wireGain=False)
wire_gain_cal.pre_collector_path = correct_for_wire_gain
wire_gain_cal.depends_on(momentum_cal)
wire_gain_cal.depends_on(cosine_cal)

correct_for_run_gain = create_path()
correct_for_run_gain.add_module('CDCDedxCorrection', scaleCor=False, momentumCor=True,
                                momentumCorFromDB=True, cosineCor=True,
                                runGain=False, wireGain=True)
run_gain_cal.pre_collector_path = correct_for_run_gain
run_gain_cal.depends_on(momentum_cal)
run_gain_cal.depends_on(cosine_cal)
run_gain_cal.depends_on(wire_gain_cal)

correct_for_others = create_path()
correct_for_others.add_module('CDCDedxCorrection', scaleCor=False, momentumCor=True,
                              momentumCorFromDB=True, cosineCor=True,
                              runGain=True, wireGain=True)
other_cal.pre_collector_path = correct_for_others
other_cal.depends_on(momentum_cal)
other_cal.depends_on(cosine_cal)
other_cal.depends_on(wire_gain_cal)
other_cal.depends_on(run_gain_cal)


# Create a CAF instance and add calibrations
caf_fw = CAF()
caf_fw.add_calibration(momentum_cal)
caf_fw.add_calibration(cosine_cal)
caf_fw.add_calibration(wire_gain_cal)
caf_fw.add_calibration(run_gain_cal)
caf_fw.add_calibration(other_cal)

# Run the calibration
caf_fw.run()  # Creates local database files when finished (no auto upload)
