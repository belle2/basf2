#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file computes PXD hot pixel masks from root formatted raw data from Onsen
# running the CAF
#
# Execute as: basf2 hotpixel_caf_onsen.py -- --filepath_pattern
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

from basf2 import *
set_log_level(LogLevel.INFO)

import pickle
import glob
import os
import ROOT
from ROOT.Belle2 import PXDHotPixelMaskCalibrationAlgorithm
from caf.framework import Calibration, CAF
from caf import backends
from caf.utils import IoV
from caf.utils import get_iov_from_file

import argparse
parser = argparse.ArgumentParser(description="Compute hot pixel masks for PXD from rawhit occupancy")
parser.add_argument('--filepath_pattern', default='', type=str, help='File path pattern')
args = parser.parse_args()


input_files = glob.glob(args.filepath_pattern)

print('Number selected input files:  {}'.format(len(input_files)))

# Create and configure the collector and its pre collector path
hotpixelcollector = register_module("PXDRawHotPixelMaskCollector")
hotpixelcollector.param("granularity", "run")

# The pre collector path must contain geometry and unpacker
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
geometry = register_module('Geometry')
geometry.param('components', ['PXD'])
pre_collector_path = create_path()
pre_collector_path.add_module(gearbox)
pre_collector_path.add_module(geometry)
pre_collector_path.add_module('PXDUnpacker')


# Create and configure the calibration algorithm
hotpixelkiller = PXDHotPixelMaskCalibrationAlgorithm()  # Getting a calibration algorithm instance
# We can play around with hotpixelkiller parameters
hotpixelkiller.minEvents = 10000         # Minimum number of events = typical size of one subrun
hotpixelkiller.minHits = 5               # Only consider pixels for masking with certain minimum number of hits
hotpixelkiller.maxOccupancy = 1e-2       # Mask pixels whose occupancy exceeds this limit
hotpixelkiller.maskDrains = False         # Set True to allow masking of hot drain lines
hotpixelkiller.minHitsDrain = 10         # Only consider drain lines for masking with certain minimum number of hits
hotpixelkiller.maxOccupancyDrain = 1e-1  # Maks drain line whose (average) occupancy exceeds this limit
hotpixelkiller.maskRows = False          # Set True to allow masking of hot rows
hotpixelkiller.minHitsRow = 10           # Only consider rows for masking with certain minimum number of hits
hotpixelkiller.maxOccupancyRow = 1e-1    # Mask row whose (average) occupancy exceeds this limit
# We want to use a specific collector collecting from raw hits
hotpixelkiller.setPrefix("PXDRawHotPixelMaskCollector")

# Create a calibration
cal = Calibration(
    name="PXDHotPixelMaskCalibrationAlgorithm",
    collector=hotpixelcollector,
    algorithms=hotpixelkiller,
    input_files=input_files)
cal.pre_collector_path = pre_collector_path


# Here we set the AlgorithmStrategy for our algorithm
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
# The SequentialRunByRun strategy executes your algorithm over runs
# individually to give you payloads for each one (if successful)
cal.strategies = SimpleRunByRun  # SequentialRunByRun

cal.max_files_per_collector_job = 1

# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
# cal_fw.backend = backends.LSF()
cal_fw.backend = backends.Local(max_processes=2)
# Time between polling checks to the CAF to see if a step (algorithm, collector jobs) is complete
cal_fw.heartbeat = 30
# Can change where your calibration runs
cal_fw.output_dir = 'calibration_results'
cal_fw.run()
# Should have created a directory called 'calibration_results'
