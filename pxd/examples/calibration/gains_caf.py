#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file computes PXD gain corrections from using bg simulations and
# beam data. The script uses the CAF framework.
#
# Collector outputs need to be prepared in advance, using script gain_collector_mc.py
# Their absolute path must be added manually to the list mc_collector_output_files.
#
# Execute as: basf2 gains_caf.py -- --data_filepath_pattern='/whatever/*.root'
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

from basf2 import *
set_log_level(LogLevel.INFO)

import pickle
import glob
import os
import ROOT
from ROOT.Belle2 import PXDGainCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf import backends
from caf.utils import IoV
from caf.utils import get_iov_from_file
from caf.utils import find_absolute_file_paths


import argparse
parser = argparse.ArgumentParser(description="Compute PXD gain calibrations from beam data runs")
parser.add_argument('--data_filepath_pattern', default='', type=str, help='File path pattern')
args = parser.parse_args()

# FIXME You need to hardcode the absolute paths for the MC collector outputs.
mc_collector_output_files = ['/home/benjamin/b2/head/pxd/examples/calibration/PXDGainCollectorOutput_MC_set0.root',
                             '/home/benjamin/b2/head/pxd/examples/calibration/PXDGainCollectorOutput_MC_set1.root',
                             '/home/benjamin/b2/head/pxd/examples/calibration/PXDGainCollectorOutput_MC_set2.root',
                             '/home/benjamin/b2/head/pxd/examples/calibration/PXDGainCollectorOutput_MC_set3.root',
                             '/home/benjamin/b2/head/pxd/examples/calibration/PXDGainCollectorOutput_MC_set4.root', ]


input_files_data = find_absolute_file_paths(glob.glob(args.data_filepath_pattern))
print('List of data input files is:  {}'.format(input_files_data))

# Create and configure the collector on beam data and its pre collector path
gaincollector_data = register_module("PXDGainCollector")
gaincollector_data.param("granularity", "run")
gaincollector_data.param("minClusterCharge", 8)
gaincollector_data.param("minClusterSize", 2)
gaincollector_data.param("maxClusterSize", 6)
gaincollector_data.param("collectSimulatedData", False)
gaincollector_data.param("nBinsU", 4)
gaincollector_data.param("nBinsV", 6)

# The pre collector path on data
pre_collector_path_data = create_path()
pre_collector_path_data.add_module("Gearbox", fileName='geometry/Beast2_phase2.xml')
pre_collector_path_data.add_module("Geometry")
pre_collector_path_data.add_module("ActivatePXDPixelMasker")
pre_collector_path_data.add_module('PXDUnpacker')
pre_collector_path_data.add_module("PXDRawHitSorter")
pre_collector_path_data.add_module("PXDClusterizer")


# Create and configure the calibration algorithm
algo = PXDGainCalibrationAlgorithm()

# We can play around with algo parameters
algo.minClusters = 1000      # Minimum number of collected clusters for estimating gains
algo.noiseSigma = 1.0        # Artificial noise sigma for smearing cluster charge

# We want to use a specific collector
algo.setPrefix("PXDGainCollector")

# Create a calibration
cal = Calibration(
    name="PXDGainCalibrationAlgorithm",
    collector=gaincollector_data,
    algorithms=algo,
    input_files=input_files_data)
cal.pre_collector_path = pre_collector_path_data


def algorithm_inputdata_setup(self, input_file_paths):
    """
    Extending the normal algorithm input file setup to also use some hardcoded files not created during the CAF running.
    We are basically patching the Algorithm class to use this function instead,

    The input files from the collector run during the CAF is the same (see caf.framework.Algorithm).
    It takes all files returned from the `Calibration.output_patterns` and filters for only the CollectorOutput.root files.
    Then it sets them as input files to the CalibrationAlgorithm class being managed.

    The extension happens when it also sets input files from the file list coming from outside.

    Parameters:
      self:             The caf.framework.Algorithm instance we are patching
      input_file_paths: The files found in the collector job output directories that matched `Calibration.output_patterns`
    """
    from caf.utils import B2INFO_MULTILINE
    from pathlib import Path

    # First we do the normal input file stuff
    collector_output_files = list(filter(lambda file_path: "CollectorOutput.root" == Path(file_path).name,
                                         input_file_paths))

    all_input_files = []
    all_input_files.extend(collector_output_files)
    # Now we can add to the input files the collector output files from the list mc_collector_output_files.
    all_input_files.extend(mc_collector_output_files)

    info_lines = ["Input files passed to algorithm {}:".format(self.name)]
    info_lines.extend(all_input_files)
    B2INFO_MULTILINE(info_lines)
    self.algorithm.setInputFileNames(all_input_files)


# Now patch it in
import functools
cal.algorithms[0].data_input = functools.partial(algorithm_inputdata_setup, cal.algorithms[0])

cal.use_central_database("Calibration_Offline_Development")

# Here we set the AlgorithmStrategy for our algorithm
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
# The SequentialRunByRun strategy executes your algorithm over runs
# individually to give you payloads for each one (if successful)
cal.strategies = SingleIOV  # SequentialRunByRun

cal.max_files_per_collector_job = 1

# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.backend = backends.Local(max_processes=2)
# Time between polling checks to the CAF to see if a step (algorithm, collector jobs) is complete
cal_fw.heartbeat = 30
# Can change where your calibration runs
cal_fw.output_dir = 'calibration_results'
cal_fw.run()
# Should have created a directory called 'calibration_results'
