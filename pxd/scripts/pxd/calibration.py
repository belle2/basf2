#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
PXD calibration APIs for CAF
Author: qingyuan.liu@desy.de
"""

from basf2 import register_module, create_path
from ROOT.Belle2 import PXDHotPixelMaskCalibrationAlgorithm
from caf.framework import Calibration
from caf.strategies import SequentialRunByRun, SimpleRunByRun

run_types = ['beam', 'physics', 'cosmic', 'all']


def hot_pixel_mask_calibration(input_files, cal_name="PXDHotPixelMaskCalibration", run_type=None, global_tags=None, local_dbs=None):
    """
    Parameters:
      input_files (list): A list of input files to be assigned to calibration.input_files

      run_type    ( str): The run type which will define different calibration parameters.
        Should be in ['beam', 'physics', 'cosmic', 'all']

      global_tags (list): A list of global tags

      local_dbs   (list): A list of local databases

    Return:
      A caf.framework.Calibration obj.
    """

    if global_tags is None:
        global_tags = []
    if local_dbs is None:
        local_dbs = []
    if run_type is None:
        run_type = 'all'
    if not isinstance(run_type, str) or run_type.lower() not in run_types:
        raise ValueError("run_type not found in run_types : {}".format(run_type))

    # Create BASF2 path

    gearbox = register_module('Gearbox')
    geometry = register_module('Geometry')
    geometry.param('components', ['PXD'])
    pxdunpacker = register_module('PXDUnpacker')
    pxdunpacker.param('SuppressErrorMask', 0xFFFFFFFFFFFFFFFF)
    checker = register_module('PXDPostErrorChecker')
    checker.param("CriticalErrorMask", 0)  # 0xC000000000000000)

    main = create_path()
    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(pxdunpacker)
    main.add_module(checker)
    main.add_module("PXDRawHitSorter")

    # Collector setup

    collector = register_module("PXDRawHotPixelMaskCollector")
    collector.param("granularity", "run")

    # Algorithm setup

    algorithm = PXDHotPixelMaskCalibrationAlgorithm()  # Getting a calibration algorithm instanced
    algorithm.forceContinueMasking = False  # Continue masking even when few/no events were collected
    algorithm.minEvents = 30000             # Minimum number of collected events for masking
    algorithm.minHits = 15                  # Do dead pixel masking when median number of hits per pixel is higher
    algorithm.pixelMultiplier = 7           # Occupancy threshold is median occupancy x multiplier
    algorithm.maskDrains = True             # Set True to allow masking of hot drain lines
    algorithm.drainMultiplier = 7           # Occupancy threshold is median occupancy x multiplier
    algorithm.maskRows = True               # Set True to allow masking of hot rows
    algorithm.rowMultiplier = 7             # Occupancy threshold is median occupancy x multiplier
    algorithm.setPrefix("PXDRawHotPixelMaskCollector")
    if run_type == 'cosmic':
        algorithm.forceContinueMasking = True

    # Create the calibration instance

    cal = Calibration(
        name=cal_name,
        collector=collector,
        algorithms=[algorithm],
        input_files=input_files)
    for global_tag in global_tags:
        cal.use_central_database(global_tag)
    for local_db in local_dbs:
        cal.use_local_database(local_db)
    cal.pre_collector_path = main
    cal.strategies = SequentialRunByRun

    # Run type dependent configurations

    if run_type == 'cosmic':
        cal.strategies = SimpleRunByRun

    return cal
