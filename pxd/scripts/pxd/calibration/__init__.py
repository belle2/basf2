#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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


def hot_pixel_mask_calibration(
        input_files,
        cal_name="PXDHotPixelMaskCalibration",
        run_type=None,
        global_tags=None,
        local_dbs=None,
        **kwargs):
    """
    Parameters:
      input_files (list): A list of input files to be assigned to calibration.input_files

      run_type    ( str): The run type which will define different calibration parameters.
        Should be in ['beam', 'physics', 'cosmic', 'all']

      global_tags (list): A list of global tags

      local_dbs   (list): A list of local databases

      **kwargs: Additional configuration to support extentions without changing scripts in calibration folder.
        Supported options are listed below:

        "activate_masking" is a boolean to activate existing masking in the payload.
          PXDHotPixelMaskCollector will be used then instead of PXDRawHotPixelMaskCollector

        "debug_hist" is the flag to save a ROOT file for debug histograms.

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
    activate_masking = kwargs.get("activate_masking", False)
    if not isinstance(activate_masking, bool):
        raise ValueError("activate_masking is not a boolean!")
    debug_hist = kwargs.get("debug_hist", True)
    if not isinstance(debug_hist, bool):
        raise ValueError("debug_hist is not a boolean!")

    # Create basf2 path

    gearbox = register_module('Gearbox')
    geometry = register_module('Geometry')
    geometry.param('components', ['PXD'])
    pxdunpacker = register_module('PXDUnpacker')
    # pxdunpacker.param('ContinueOnError', True)
    checker = register_module('PXDPostErrorChecker')
    # checker.param("CriticalErrorMask", 0)

    main = create_path()
    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(pxdunpacker)
    main.add_module(checker)
    if activate_masking:
        main.add_module("ActivatePXDPixelMasker")
    main.add_module("PXDRawHitSorter")

    # Collector setup
    collector_name = "PXDRawHotPixelMaskCollector"
    if activate_masking:
        collector_name = "PXDHotPixelMaskCollector"
    collector = register_module(collector_name)
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
    algorithm.setDebugHisto(debug_hist)
    algorithm.setPrefix(collector_name)
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
                     gain_method="Analytic", validation=True, **kwargs):
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

      **kwargs: Additional configuration to support extentions without changing scripts in calibration folder.
        Supported options are listed below:

        "collector_prefix" is a string indicating which collector to be used for gain calibration. The supported
          collectors are:
            PXDPerformanceVariablesCollector (default),
            PXDPerformanceCollector(using RAVE package for vertexing, obsolete)

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
    collector_prefix = kwargs.get("collector_prefix", "PXDPerformanceVariablesCollector")
    supported_collectors = ["PXDPerformanceVariablesCollector", "PXDPerformanceCollector"]
    if not isinstance(collector_prefix, str) or collector_prefix not in supported_collectors:
        raise ValueError("collector_prefix not found in {}".format(supported_collectors))

    # Create basf2 path

    gearbox = register_module('Gearbox')
    geometry = register_module('Geometry', useDB=True)
    genFitExtrapolation = register_module('SetupGenfitExtrapolation')
    roiFinder = register_module('PXDROIFinder')
    # roiFinder.param('tolerancePhi', 0.15)  # default: 0.15
    # roiFinder.param('toleranceZ', 0.5)  # default 0.5

    main = create_path()
    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(genFitExtrapolation)
    main.add_module(roiFinder)  # for PXDIntercepts
    main.add_module("ActivatePXDPixelMasker")

    # Collector setup
    collector = register_module(collector_prefix)
    if collector_prefix == "PXDPerformanceCollector":
        main.add_module("PXDPerformance")
        collector.param("fillEventTree", False)
    else:  # the default PXDPerformanceVariablesCollector
        import modularAnalysis as ana
        import vertex
        from variables import variables as vm
        # Particle list for gain calibration
        ana.fillParticleList('e+:gain', "p > 1.0", path=main)
        # Particle list for event selection in efficiency monitoring
        ana.fillParticleList('e+:eff', "pt > 2.0", path=main)
        ana.reconstructDecay('vpho:eff -> e+:eff e-:eff', '9.5<M<11.5', path=main)
        # Particle list for studying impact parameter resolution
        # vm.addAlias("pBetaSinTheta3o2", "formula(pt * (1./(1. + tanLambda**2)**0.5)**0.5)")
        # vm.addAlias("absLambda", "abs(atan(tanLambda))")
        mySelection = 'pt>1.0 and abs(dz)<1.0 and dr<0.3'
        mySelection += ' and nCDCHits>20 and nSVDHits>=8 and nPXDHits>=1'
        mySelection += ' and [abs(atan(tanLambda)) < 0.5]'
        mySelection += ' and [formula(pt * (1./(1. + tanLambda**2)**0.5)**0.5) > 2.0]'
        # mySelection += ' and [absLambda<0.5]'
        # mySelection += ' and [pBetaSinTheta3o2>2.0]'
        ana.fillParticleList('e+:res', mySelection, path=main)
        ana.reconstructDecay('vpho:res -> e+:res e-:res', '9.5<M<11.5', path=main)
        # Remove multiple candidate events
        ana.applyCuts('vpho:res', 'nParticlesInList(vpho:res)==1', path=main)
        vertex.kFit('vpho:res', conf_level=0.0, path=main)

        collector.param("PList4GainName", "e+:gain")
        collector.param("PList4EffName", "vpho:eff")
        collector.param("PList4ResName", "vpho:res")

    collector.param("granularity", "run")
    collector.param("minClusterCharge", 8)
    collector.param("minClusterSize", 1)
    collector.param("maxClusterSize", 10)
    collector.param("nBinsU", 4)
    collector.param("nBinsV", 6)
    collector.param("fillChargeRatioHistogram", True)

    # Algorithm setup
    algorithms = []
    if validation:
        validation_alg = PXDValidationAlgorithm()
        validation_alg.setPrefix(collector_prefix)
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
        algorithm.setPrefix(collector_prefix)
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
