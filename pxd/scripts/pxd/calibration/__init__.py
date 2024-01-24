#!/usr/bin/env python3

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
        "inefficientPixelMultiplier" is the multiplier on median occupancy to define an inefficient pixel.
          0 means only dead pixels are marked as dead.
          A value > 0 means any pixels with occupancy below the threshold will be marked as dead.
        "minInefficientPixels" is the minimum number of pixels to define a dead or inefficient row.
          The rows with inefficient pixels >= this value will be marked as dead rows for now.
        "deadPixelPayloadName" is the payload name used for more defective pixel types. The default is PXDDeadPixelPar.

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
        raise ValueError(f"run_type not found in run_types : {run_type}")
    activate_masking = kwargs.get("activate_masking", False)
    if not isinstance(activate_masking, bool):
        raise ValueError("activate_masking is not a boolean!")
    debug_hist = kwargs.get("debug_hist", True)
    if not isinstance(debug_hist, bool):
        raise ValueError("debug_hist is not a boolean!")
    inefficientPixelMultiplier = kwargs.get("inefficientPixelMultiplier", 0.)
    if not isinstance(inefficientPixelMultiplier, float):
        raise ValueError("inefficientPixelMultiplier is not a float!")
    minInefficientPixels = kwargs.get("minInefficientPixels", 250)
    if not isinstance(minInefficientPixels, int):
        raise ValueError("minInefficientPixels is not an int!")
    deadPixelPayloadName = kwargs.get("deadPixelPayloadName", "PXDDeadPixelPar")
    if not isinstance(deadPixelPayloadName, str):
        raise ValueError("deadPixelPayloadName is not a str!")

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
    # Occupancy threshold is median occupancy x multiplier
    algorithm.inefficientPixelMultiplier = inefficientPixelMultiplier
    # Minimum number of inefficient pixels in a dead/inefficient row
    algorithm.minInefficientPixels = minInefficientPixels
    algorithm.deadPixelPayloadName = deadPixelPayloadName
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

        "collector_prefix": a string indicating which collector to be used for gain calibration. The supported
          collectors are:
            PXDPerformanceVariablesCollector (default),
            PXDPerformanceCollector(using RAVE package for vertexing, obsolete)
        "useClusterPosition": Flag to use cluster postion rather than track point to group pixels for calibration.
        "particle_type": Particle type assigned to tracks. "e" by default.
        "track_cuts_4gain": Track cuts used for gain calibration.
        "track_cuts_4eff": Track cuts used for efficiency study.
        "track_cuts_4res": Track cuts used for resolution study.

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
        raise ValueError(f"gain_method not found in gain_methods : {gain_method}")
    collector_prefix = kwargs.get("collector_prefix", "PXDPerformanceVariablesCollector")
    supported_collectors = ["PXDPerformanceVariablesCollector", "PXDPerformanceCollector"]
    if not isinstance(collector_prefix, str) or collector_prefix not in supported_collectors:
        raise ValueError(f"collector_prefix not found in {supported_collectors}")
    useClusterPosition = kwargs.get("useClusterPosition", False)
    if not isinstance(useClusterPosition, bool):
        raise ValueError("useClusterPosition has to be a boolean!")
    particle_type = kwargs.get("particle_type", "e")  # rely on modular analysis for value check
    track_cuts_4gain = kwargs.get("track_cuts_4gain", "p > 1.0")  # see above
    track_cuts_4eff = kwargs.get("track_cuts_4eff", "pt > 2.0")  # see above
    track_cuts_4res = kwargs.get("track_cuts_4res", "Note2019")  # NOTE-TE-2019-018

    # Create basf2 path

    gearbox = register_module('Gearbox')
    geometry = register_module('Geometry', useDB=True)
    genFitExtrapolation = register_module('SetupGenfitExtrapolation')
    roiFinder = register_module('PXDROIFinder')

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
        # Particle list for gain calibration
        p = particle_type
        ana.fillParticleList(f'{p}+:gain', track_cuts_4gain, path=main)

        # Particle list for event selection in efficiency monitoring
        # nSVDHits > 5 doesn't help, firstSVDLayer == 3 for < 0.1% improvement?
        ana.fillParticleList(f'{p}+:eff', track_cuts_4eff, path=main)
        # Mass cut (9.5 < M < 11.5) below can improve efficiency by ~ 1%
        ana.reconstructDecay(f'vpho:eff -> {p}+:eff {p}-:eff', '9.5<M<11.5', path=main)
        # < 0.1% improvement by using kfit pvalue >= 0.01 after mass cut
        # vertex.kFit('vpho:eff', conf_level=0.01, fit_type="fourC", daughtersUpdate=False, path=main)

        # Particle list for studying impact parameter resolution
        # Alias dosn't work with airflow implementation
        # from variables import variables as vm
        # vm.addAlias("pBetaSinTheta3o2", "formula(pt * (1./(1. + tanLambda**2)**0.5)**0.5)")
        # vm.addAlias("absLambda", "abs(atan(tanLambda))")
        track_cuts_4res_note2019 = 'pt>1.0 and abs(dz)<1.0 and dr<0.3'
        track_cuts_4res_note2019 += ' and nCDCHits>20 and nSVDHits>=8 and nPXDHits>=1'
        track_cuts_4res_note2019 += ' and [abs(atan(tanLambda)) < 0.5]'
        track_cuts_4res_note2019 += ' and [formula(pt * (1./(1. + tanLambda**2)**0.5)**0.5) > 2.0]'
        # track_cuts_4res_note2019 += ' and [absLambda<0.5]'
        # track_cuts_4res_note2019 += ' and [pBetaSinTheta3o2>2.0]'
        if track_cuts_4res == "Note2019":
            track_cuts_4res = track_cuts_4res_note2019
        ana.fillParticleList(f'{p}+:res', track_cuts_4res, path=main)
        ana.reconstructDecay(f'vpho:res -> {p}+:res {p}-:res', '9.5<M<11.5', path=main)
        # Remove multiple candidate events
        ana.applyCuts('vpho:res', 'nParticlesInList(vpho:res)==1', path=main)
        vertex.kFit('vpho:res', conf_level=0.0, path=main)

        collector.param("PList4GainName", f"{p}+:gain")
        collector.param("PList4EffName", "vpho:eff")
        collector.param("PList4ResName", "vpho:res")
        collector.param("maskedDistance", 3)
        collector.param("useClusterPosition", useClusterPosition)

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
        validation_alg.saveD0Z0 = kwargs.get("saveD0Z0", False)  # Flag to save corrected d0 (z0) difference
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
