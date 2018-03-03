#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file shows pretty much the most minimal setup for
# running the CAF. If we had an algorithm that needed iteration this
# would be done automatically. Other features include:
#   * Chaining multiple calibrations together with arbitrary dependencies.
#   * Running collectors on a batch system backend.
#   * pre-collector module path to setup the datastore ready for your collector.
#   * pre-algorithm setup function to setup the environment for the algorithm each
#     time it's executed.
#   * By default we iterate over all runs and merge if not enough data, use the
#     collector module parameter granularity = 'all' to get one big IoV for all data

from basf2 import *
set_log_level(LogLevel.INFO)

import glob

import ROOT
from ROOT.Belle2 import PXDHotPixelMaskCalibrationAlgorithm
from caf.framework import Calibration, CAF

# input_files = glob.glob("/home/benjamin/GCR17/root/e0002r00451/*.root")
input_files = glob.glob("PXD_DST_e0002r00451.root")

# Your input files shouldn't be relative paths (for now)
input_files = [os.path.abspath(path) for path in input_files]
print(input_files)

hotpixelkiller = PXDHotPixelMaskCalibrationAlgorithm()  # Getting a calibration algorithm instance
# We can play around with hotpixelkiller parameters
hotpixelkiller.minEvents = 100000        # Minimum number of events
hotpixelkiller.minHits = 5               # Hot pixels have at least this numner of hits in all events
hotpixelkiller.maxOccupancy = 0.0        # Hot pixels occupancy exceeds this limit
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

# Adding in setup basf2 paths and functions for the collector and algorithm
# rootinput = register_module('RootInput')
# pxdunpacker = register_module('PXDUnpacker')
gearbox = register_module('Gearbox')
gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
geometry = register_module('Geometry')
geometry.param('components', ['PXD'])
pre_collector_path = create_path()
pre_collector_path.add_module(gearbox)
pre_collector_path.add_module(geometry)
# pre_collector_path.add_module(rootinput)
# pre_collector_path.add_module(pxdunpacker)
cal.pre_collector_path = pre_collector_path

main = create_path()
main.add_module('RootInput', branchNames=['PXDRawHits'])
main.add_module('HistoManager', histoFileName='RawCollectorOutput.root')
main.add_module(gearbox)
main.add_module(geometry)


# Create a CAF instance and add the calibration to it.
cal_fw = CAF()
cal_fw.add_calibration(cal)
cal_fw.run()
# Should have created a directory called 'calibration_results'
