#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file computes PXD hot pixel masks from root formatted raw data
# running the CAF
#
# Execute as: basf2 hotpixel_caf.py -- --input '/home/benjamin/GCR17/root/e0002r00451/*.root'
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

from basf2 import *
set_log_level(LogLevel.INFO)

import glob
import ROOT
from ROOT.Belle2 import PXDHotPixelMaskCalibrationAlgorithm
from caf.framework import Calibration, CAF

import argparse
parser = argparse.ArgumentParser(description="Compute hot pixel masks for PXD from rawhit occupancy")
parser.add_argument('--input', default='', type=str, help='String specifying inputfiles')
args = parser.parse_args()


input_files = glob.glob(args.input)

# Your input files shouldn't be relative paths (for now)
input_files = [os.path.abspath(path) for path in input_files]
print(input_files)

hotpixelkiller = PXDHotPixelMaskCalibrationAlgorithm()  # Getting a calibration algorithm instance
# We can play around with hotpixelkiller parameters
hotpixelkiller.minEvents = 30000         # Minimum number of events = typical size of one subrun
hotpixelkiller.minHits = 5               # Only consider pixels for masking with certain minimum number of hits
hotpixelkiller.maxOccupancy = 1e-5       # Mask pixels whose occupancy exceeds this limit
hotpixelkiller.maskDrains = True         # Set True to allow masking of hot drain lines
hotpixelkiller.minHitsDrain = 10         # Only consider drain lines for masking with certain minimum number of hits
hotpixelkiller.maxOccupancyDrain = 1e-7  # Maks drain line whose (average) occupancy exceeds this limit
hotpixelkiller.maskRows = True           # Set True to allow masking of hot rows
hotpixelkiller.minHitsRow = 10           # Only consider rows for masking with certain minimum number of hits
hotpixelkiller.maxOccupancyRow = 1e-6    # Mask row whose (average) occupancy exceeds this limit
# We want to use a specific collector
hotpixelkiller.setPrefix("PXDRawHotPixelMaskCollector")


# Create a single calibration from a collector module name + algorithm + input files
# Alternatively you could register the collector first, alter it's parameters and pass that in
hotpixelcollector = register_module("PXDRawHotPixelMaskCollector")
hotpixelcollector.param("granularity", "all")
cal = Calibration(
    name="PXDHotPixelMaskCalibrationAlgorithm",
    collector=hotpixelcollector,
    algorithms=hotpixelkiller,
    input_files=input_files)

# Adding in setup basf2 paths and functions for the collector
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
geometry = register_module('Geometry')
geometry.param('components', ['PXD'])
pre_collector_path = create_path()
pre_collector_path.add_module(gearbox)
pre_collector_path.add_module(geometry)
pre_collector_path.add_module('PXDUnpacker')
cal.pre_collector_path = pre_collector_path


# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.run()
# Should have created a directory called 'calibration_results'
