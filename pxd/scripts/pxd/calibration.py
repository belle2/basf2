#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
PXD calibration APIs for CAF
Author: qingyuan.liu@desy.de
"""

from basf2 import register_module, create_path
from ROOT.Belle2 import PXDHotPixelMaskCalibrationAlgorithm, PXDAnalyticGainCalibrationAlgorithm
from ROOT.Belle2 import PXDValidationAlgorithm
from caf.framework import Calibration
from caf.strategies import SequentialRunByRun, SimpleRunByRun

run_types = ['beam', 'physics', 'cosmic', 'all']
gain_methods = ['analytic', 'generic_mc', 'cluster_sim', '']


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
    if run_type.lower() == 'cosmic':
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

    if run_type.lower() == 'cosmic':
        cal.strategies = SimpleRunByRun

    return cal


def gain_calibration(input_files, cal_name="PXDGainCalibration",
                     boundaries=None, global_tags=None, local_dbs=None,
                     gain_method="Analytic", validation=True):
    """
    Parameters:
      input_files (list): A list of input files to be assigned to calibration.input_files

      boundaries (c++ std::vector): boundaries for iov creation

      global_tags (list): A list of global tags

      local_dbs   (list): A list of local databases

      gain_method (str): A string of gain algorithm in
        ['analytic', 'generic_mc', 'cluster_sim', '']. Empty str means to skip gain
        calibration and validation has to be True otherwise no algorithm will be used.
        Caveat: Only the analytic method is now implemented.

      validation (bool): Adding validation algorithm if True (default)

    Return:
      A caf.framework.Calibration obj.
    """

    if global_tags is None:
        global_tags = []
    if local_dbs is None:
        local_dbs = []
    if gain_method is None:
        gain_method = 'analytic'
    if not isinstance(gain_method, str) or gain_method.lower() not in gain_methods:
        raise ValueError("gain_method not found in gain_methods : {}".format(gain_method))

    # Create BASF2 path

    gearbox = register_module('Gearbox')
    geometry = register_module('Geometry', useDB=True)
    # pxdunpacker = register_module('PXDUnpacker')
    # pxdunpacker.param('SuppressErrorMask', 0xFFFFFFFFFFFFFFFF)
    # checker = register_module('PXDPostErrorChecker')
    # checker.param("CriticalErrorMask", 0)  # 0xC000000000000000)
    genFitExtrapolation = register_module('SetupGenfitExtrapolation')
    roiFinder = register_module('PXDROIFinder')
    # roiFinder.param('tolerancePhi', 0.15)  # default: 0.15
    # roiFinder.param('toleranceZ', 0.5)  # default 0.5

    main = create_path()
    main.add_module(gearbox)
    main.add_module(geometry)
    # main.add_module(pxdunpacker)
    # main.add_module(checker)
    # main.add_module("PXDRawHitSorter")
    main.add_module(genFitExtrapolation)
    main.add_module(roiFinder)  # for PXDIntercepts
    main.add_module("PXDPerformance")

    # Collector setup

    collector = register_module("PXDPerformanceCollector")
    collector.param("granularity", "run")
    collector.param("minClusterCharge", 8)
    collector.param("minClusterSize", 1)
    collector.param("maxClusterSize", 10)
    collector.param("nBinsU", 4)
    collector.param("nBinsV", 6)
    collector.param("fillEventTree", False)
    collector.param("fillChargeRatioHistogram", True)

    # Algorithm setup
    algorithms = []
    if validation:
        validation_alg = PXDValidationAlgorithm()
        validation_alg.setPrefix("PXDPerformanceCollector")
        validation_alg.minTrackPoints = 10     # Minimum number of track points
        validation_alg.save2DHists = True      # Flag to save 2D histogram for efficiency on layer 1 or 2 in Z-phi plane
        algorithms.append(validation_alg)

    # validation_alg.setBoundaries(boundaries)  # This takes boundaries from the expert_config
    if (gain_method != ''):
        algorithm = PXDAnalyticGainCalibrationAlgorithm()  # Getting a calibration algorithm instanced
        algorithm.minClusters = 200            # Minimum number of clusters in each region
        algorithm.safetyFactor = 11            # Safety factor x minClusters/region -> <minClusters> for module
        algorithm.forceContinue = False        # Force continue the algorithm instead of return c_notEnoughData
        algorithm.strategy = 0                 # 0: medians, 1: landau fit
        algorithm.correctForward = True        # Correct default gains in forward regions due to low statistics
        algorithm.useChargeRatioHistogram = True  # Use Charge ratio histograms for the calibration
        algorithm.setPrefix("PXDPerformanceCollector")
        # algorithm.setBoundaries(boundaries)  # This takes boundaries from the expert_config
        algorithms.append(algorithm)

    # Create the calibration instance

    cal = Calibration(
        name=cal_name,
        collector=collector,
        algorithms=algorithms,
        input_files=input_files)
    for global_tag in global_tags:
        cal.use_central_database(global_tag)
    for local_db in local_dbs:
        cal.use_local_database(local_db)
    cal.pre_collector_path = main
    # cal.strategies = SequentialBoundaries
    cal.strategies = SequentialRunByRun
    # cal.strategies = SimpleRunByRun

    return cal
