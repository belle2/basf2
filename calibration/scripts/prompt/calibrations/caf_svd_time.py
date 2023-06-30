#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
Script to perform the svd time calibration with the CoG6, CoG3 and ELS3 algorithms
'''

import sys
import datetime
import random

from ROOT.Belle2 import SVDCoGTimeCalibrationAlgorithm
from ROOT.Belle2 import SVD3SampleCoGTimeCalibrationAlgorithm
from ROOT.Belle2 import SVD3SampleELSTimeCalibrationAlgorithm
from ROOT.Belle2 import SVDTimeValidationAlgorithm

import basf2 as b2

import rawdata as raw
from softwaretrigger.constants import HLT_INPUT_OBJECTS
from tracking import add_tracking_reconstruction

from caf.framework import Calibration
from caf import strategies
from caf.utils import IoV
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from prompt.utils import filter_by_max_events_per_run

b2.set_log_level(b2.LogLevel.INFO)

random.seed(42)

now = datetime.datetime.now()

settings = CalibrationSettings(name="caf_svd_time",
                               expert_username="gdujany",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hadron_calib"],
                               input_data_filters={"hadron_calib": [INPUT_DATA_FILTERS["Data Tag"]["hadron_calib"],
                                                                    INPUT_DATA_FILTERS["Beam Energy"]["4S"],
                                                                    INPUT_DATA_FILTERS["Beam Energy"]["Continuum"],
                                                                    INPUT_DATA_FILTERS["Run Type"]["physics"],
                                                                    INPUT_DATA_FILTERS["Magnet"]["On"]]},
                               depends_on=[],
                               expert_config={
                                   "timeAlgorithms": ["CoG3", "ELS3"],
                                   "max_events_per_run": 50000,
                                   "max_events_per_file": 5000,
                                   "isMC": False,
                                   "linearCutsOnCoG3": False,
                                   "upperLineParameters": [-94.0, 1.264],
                                   "lowerLineParameters": [-134.0, 1.264],
                                   "rangeRawTimeForIoVCoG6": [20., 80.],
                                   "rangeRawTimeForIoVCoG3": [70., 140.],
                                   "rangeRawTimeForIoVELS3": [20., 80.],
                                   "useRawtimeForTracking": False,
                                   "useSVDGrouping": True
                               })

##################################################################
# Remove Module from the Path


def remove_module(path, name):

    new_path = b2.create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path
##########################################################################


NEW_RECO_DIGITS_NAME = "SVDRecoDigitsFromTracks"
NEW_SHAPER_DIGITS_NAME = "SVDShaperDigitsFromTracks"


def create_collector(name="SVDTimeCalibrationCollector",
                     clusters="SVDClustersFromTracks",
                     event_info="SVDEventInfo",
                     event_t0="EventT0",
                     rawBinWidth=2,
                     granularity="run",
                     minRawTimeForIoV=0.,
                     maxRawTimeForIoV=150.):
    """
    Simply creates a SVDTimeCalibrationCollector module with some options.

    Returns:
        pybasf2.Module
    """

    collector = b2.register_module("SVDTimeCalibrationCollector")
    collector.set_name(name)
    collector.param("SVDClustersFromTracksName", clusters)
    collector.param("SVDEventInfoName", event_info)
    collector.param("EventT0Name", event_t0)
    collector.param("granularity", granularity)
    collector.param("RawCoGBinWidth", rawBinWidth)
    collector.param("RawTimeIoVMin", minRawTimeForIoV)
    collector.param("RawTimeIoVMax", maxRawTimeForIoV)

    return collector


def create_validation_collector(name="SVDTimeValidationCollector",
                                clusters="SVDClusters",
                                clusters_onTracks="SVDClustersOnTracks",
                                event_t0="EventT0",
                                granularity="run"):
    """
    Simply creates a SVDTimeCalibrationCollector module with some options.

    Returns:
        pybasf2.Module
    """

    collector = b2.register_module("SVDTimeValidationCollector")
    collector.set_name(name)
    collector.param("SVDClustersName", clusters)
    collector.param("SVDClustersOnTracksName", clusters_onTracks)
    collector.param("EventT0Name", event_t0)
    collector.param("granularity", granularity)

    return collector


def create_algorithm(
        unique_id,
        prefix="",
        min_entries=10000,
        linearCutsOnCoG3=False,
        interceptUpperLine=-94.0,
        angularCoefficientUpperLine=1.264,
        interceptLowerLine=-134.0,
        angularCoefficientLowerLine=1.264):
    """
    Simply creates a SVDCoGTimeCalibrationAlgorithm class with some options.

    Returns:
        ROOT.Belle2.SVDCoGTimeCalibrationAlgorithm
    """
    if "CoG6" in prefix:
        algorithm = SVDCoGTimeCalibrationAlgorithm(unique_id)
    if "CoG3" in prefix:
        algorithm = SVD3SampleCoGTimeCalibrationAlgorithm(unique_id)
        algorithm.setTwoLineSelectionParameters(
            linearCutsOnCoG3,
            interceptUpperLine,
            angularCoefficientUpperLine,
            interceptLowerLine,
            angularCoefficientLowerLine)
    if "ELS3" in prefix:
        algorithm = SVD3SampleELSTimeCalibrationAlgorithm(unique_id)
    if prefix:
        algorithm.setPrefix(prefix)
    algorithm.setMinEntries(min_entries)

    return algorithm


def create_validation_algorithm(prefix="", min_entries=10000):
    """
    Simply creates a SVDCoGTimeValidationAlgorithm class with some options.

    Returns:
        ROOT.Belle2.SVDCoGTimeValidationAlgorithm
    """
    algorithm = SVDTimeValidationAlgorithm()
    if prefix:
        algorithm.setPrefix(prefix)
    algorithm.setMinEntries(min_entries)

    return algorithm


def create_svd_clusterizer(name="ClusterReconstruction",
                           clusters="SVDClustersFromTracks",
                           reco_digits=None,
                           shaper_digits=None,
                           time_algorithm="CoG6",
                           get_3sample_raw_time=False):
    """
    Simply creates a SVDClusterizer module with some options.

    Returns:
        pybasf2.Module
    """

    cluster = b2.register_module("SVDClusterizer")
    cluster.set_name(name)
    cluster.param("Clusters", clusters)
    if shaper_digits is not None:
        cluster.param("ShaperDigits", shaper_digits)
    cluster.param("timeAlgorithm6Samples", time_algorithm)
    cluster.param("useDB", False)
    if get_3sample_raw_time:
        cluster.param("returnClusterRawTime", True)
    return cluster


def create_pre_collector_path(
        clusterizers,
        isMC=False,
        max_events_per_run=10000,
        max_events_per_file=10000,
        useSVDGrouping=True,
        useRawtimeForTracking=False,
        is_validation=False):
    """
    Create a basf2 path that runs a common reconstruction path and also runs several SVDSimpleClusterizer
    modules with different configurations. This way they re-use the same reconstructed objects.

    Parameters:
        clusterizers (list[pybasf2.Module]): All the differently configured
            SVDSimpleClusterizer modules. They should output to different datastore objects.
        max_events_per_run (int, optional): Max events read per run. Defaults to 10000.
        is_validation (bool, optional): Is used to produce the validation plots. Defaults to False.

    returns:
        pybasf2.Path
    """
    # Set-up re-processing path
    path = b2.create_path()

    # Read from file only what is needed
    if not isMC:
        path.add_module("RootInput", branchNames=HLT_INPUT_OBJECTS, entrySequences=[f'0:{max_events_per_file - 1}'])
    else:
        path.add_module("RootInput")

    # unpack raw data to do the tracking
    if not isMC:
        raw.add_unpackers(path, components=['PXD', 'SVD', 'CDC'])
    else:
        path.add_module("Gearbox")
        path.add_module("Geometry")

    # proceed only if we acquired 6-sample strips
    skim6SampleEvents = b2.register_module("SVD6SampleEventSkim")
    path.add_module(skim6SampleEvents)
    emptypath = b2.create_path()
    skim6SampleEvents.if_false(emptypath)

    if not isMC:
        # run tracking reconstruction
        add_tracking_reconstruction(path, append_full_grid_cdc_eventt0=True)
        path = remove_module(path, "V0Finder")
        if not is_validation:
            # if we would like using the grouping (True by default), we should use the calibrated cluster time
            # if we would like to use the raw time, than the cluster grouping parameters are OK only for CoG3
            # in the reconstruction (default in reconstruction)
            if useSVDGrouping:
                if useRawtimeForTracking:
                    b2.set_module_parameters(path, 'SVDClusterizer', returnClusterRawTime=True)
                    b2.set_module_parameters(path, 'SVDTimeGrouping', useDB=False,
                                             isEnabledIn6Samples=True,
                                             acceptSigmaN=5, removeSigmaN=5,
                                             expectedSignalTimeCenter=100,
                                             expectedSignalTimeMin=70, expectedSignalTimeMax=130,
                                             tRangeLow=-20, tRangeHigh=220,
                                             numberOfSignalGroups=2, formSingleSignalGroup=True)
                else:
                    b2.set_module_parameters(path, 'SVDTimeGrouping', useDB=False,
                                             isEnabledIn6Samples=True, acceptSigmaN=3,
                                             expectedSignalTimeMin=-30, expectedSignalTimeMax=30)
                b2.set_module_parameters(path, 'SVDSpacePointCreator', useDB=False,
                                         useSVDGroupInfoIn6Sample=True)
            else:
                b2.set_module_parameters(path, 'SVDClusterizer', returnClusterRawTime=True)
                b2.set_module_parameters(path, 'SVDTimeGrouping', useDB=False, isEnabledIn6Samples=False)

        # repeat svd reconstruction using only SVDShaperDigitsFromTracks
        path.add_module("SVDShaperDigitsFromTracks")

    for cluster in clusterizers:
        path.add_module(cluster)

    path = remove_module(path, "SVDMissingAPVsClusterCreator")

    return path


def get_calibrations(input_data, **kwargs):

    file_to_iov_physics = input_data["hadron_calib"]
    expert_config = kwargs.get("expert_config")
    timeAlgorithms = expert_config["timeAlgorithms"]
    max_events_per_run = expert_config["max_events_per_run"]  # Maximum number of events selected per each run
    max_events_per_file = expert_config["max_events_per_file"]  # Maximum number of events selected per each file
    isMC = expert_config["isMC"]
    linearCutsOnCoG3 = expert_config["linearCutsOnCoG3"]
    upperLineParameters = expert_config["upperLineParameters"]
    lowerLineParameters = expert_config["lowerLineParameters"]
    rangeRawTimeForIoVCoG6 = expert_config["rangeRawTimeForIoVCoG6"]
    rangeRawTimeForIoVCoG3 = expert_config["rangeRawTimeForIoVCoG3"]
    rangeRawTimeForIoVELS3 = expert_config["rangeRawTimeForIoVELS3"]
    useSVDGrouping = expert_config["useSVDGrouping"]
    useRawtimeForTracking = expert_config["useRawtimeForTracking"]

    reduced_file_to_iov_physics = filter_by_max_events_per_run(file_to_iov_physics,
                                                               max_events_per_run, random_select=True,
                                                               max_events_per_file=max_events_per_file)
    good_input_files = list(reduced_file_to_iov_physics.keys())

    b2.B2INFO(f"Total number of files used as input = {len(good_input_files)}")

    exps = [i.exp_low for i in reduced_file_to_iov_physics.values()]
    runs = sorted([i.run_low for i in reduced_file_to_iov_physics.values()])

    firstRun = runs[0]
    lastRun = runs[-1]
    expNum = exps[0]

    if not len(good_input_files):
        print("No good input files found! Check that the input files have entries != 0!")
        sys.exit(1)

    cog6_suffix = "_CoG6"
    cog3_suffix = "_CoG3"
    els3_suffix = "_ELS3"

    calType = "Prompt"
    if isMC:
        calType = "MC"
    unique_id_cog6 = f"SVDCoGTimeCalibrations_{calType}_{now.isoformat()}_INFO:_3rdOrderPol_TBindep_" \
                     f"Exp{expNum}_runsFrom{firstRun}to{lastRun}"
    print(f"\nUniqueID_CoG6:\n{unique_id_cog6}")

    unique_id_cog3 = f"SVD3SampleCoGTimeCalibrations_{calType}_{now.isoformat()}_INFO:_3rdOrderPol_TBindep_" \
                     f"Exp{expNum}_runsFrom{firstRun}to{lastRun}"
    print(f"\nUniqueID_CoG3:\n{unique_id_cog3}")

    unique_id_els3 = f"SVD3SampleELSTimeCalibrations_{calType}_{now.isoformat()}_INFO:_TBindep_" \
                     f"Exp{expNum}_runsFrom{firstRun}to{lastRun}"
    print(f"\nUniqueID_ELS3:\n{unique_id_els3}")

    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ####
    # Build the clusterizers with the different options.
    ####

    cog6 = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog6_suffix}",
        clusters=f"SVDClustersFromTracks{cog6_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm="CoG6",
        get_3sample_raw_time=True)

    cog3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog3_suffix}",
        clusters=f"SVDClustersFromTracks{cog3_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm="CoG3",
        get_3sample_raw_time=True)

    els3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{els3_suffix}",
        clusters=f"SVDClustersFromTracks{els3_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm="ELS3",
        get_3sample_raw_time=True)

    ####
    # Build the Collectors and Algorithms with the different (but matching) options
    ####
    eventInfo = "SVDEventInfo"
    if isMC:
        eventInfo = "SVDEventInfoSim"

    coll_cog6 = create_collector(
        name=f"SVDTimeCalibrationCollector{cog6_suffix}",
        clusters=f"SVDClustersFromTracks{cog6_suffix}",
        event_info=eventInfo,
        event_t0="EventT0",
        minRawTimeForIoV=rangeRawTimeForIoVCoG6[0],
        maxRawTimeForIoV=rangeRawTimeForIoVCoG6[1])

    algo_cog6 = create_algorithm(
        unique_id_cog6,
        prefix=coll_cog6.name(),
        min_entries=10000)

    coll_cog3 = create_collector(
        name=f"SVDTimeCalibrationCollector{cog3_suffix}",
        clusters=f"SVDClustersFromTracks{cog3_suffix}",
        event_info=eventInfo,
        rawBinWidth=1,
        event_t0="EventT0",
        minRawTimeForIoV=rangeRawTimeForIoVCoG3[0],
        maxRawTimeForIoV=rangeRawTimeForIoVCoG3[1])

    algo_cog3 = create_algorithm(
        unique_id_cog3,
        prefix=coll_cog3.name(),
        min_entries=10000,
        linearCutsOnCoG3=linearCutsOnCoG3,
        interceptUpperLine=upperLineParameters[0],
        angularCoefficientUpperLine=upperLineParameters[1],
        interceptLowerLine=lowerLineParameters[0],
        angularCoefficientLowerLine=lowerLineParameters[1])

    coll_els3 = create_collector(
        name=f"SVDTimeCalibrationCollector{els3_suffix}",
        clusters=f"SVDClustersFromTracks{els3_suffix}",
        event_info=eventInfo,
        event_t0="EventT0",
        minRawTimeForIoV=rangeRawTimeForIoVELS3[0],
        maxRawTimeForIoV=rangeRawTimeForIoVELS3[1])

    algo_els3 = create_algorithm(
        unique_id_els3,
        prefix=coll_els3.name(),
        min_entries=10000)

    ####
    # Build the pre_collector_path for reconstruction BUT we also sneakily
    # add the two cog collectors to it.
    ####

    # Check what time algorithms (CoG6, CoG3, ELS3) will be calibrated according to expert input
    list_of_clusterizers = []
    list_of_algorithms = []
    for a in timeAlgorithms:
        if a == "CoG3":
            list_of_clusterizers.append(cog3)
            list_of_algorithms.append(algo_cog3)
        if a == "CoG6":
            list_of_clusterizers.append(cog6)
            list_of_algorithms.append(algo_cog6)
        if a == "ELS3":
            list_of_clusterizers.append(els3)
            list_of_algorithms.append(algo_els3)

    pre_collector_path = create_pre_collector_path(
        clusterizers=list_of_clusterizers,
        isMC=isMC, max_events_per_run=max_events_per_run,
        max_events_per_file=max_events_per_file,
        useSVDGrouping=useSVDGrouping, useRawtimeForTracking=useRawtimeForTracking)

    # Decide what collector will be "managed" by the CAF
    collector_managed_by_CAF = coll_els3
    if "ELS3" in timeAlgorithms:
        collector_managed_by_CAF = coll_els3
        if "CoG6" in timeAlgorithms:
            pre_collector_path.add_module(coll_cog6)
        if "CoG3" in timeAlgorithms:
            pre_collector_path.add_module(coll_cog3)
    # We leave the coll_els3 to be the one "managed" by the CAF

    elif "CoG3" in timeAlgorithms:
        collector_managed_by_CAF = coll_cog3
        if "CoG6" in timeAlgorithms:
            pre_collector_path.add_module(coll_cog6)
    else:
        collector_managed_by_CAF = coll_cog6

    # calibration setup
    calibration = Calibration("SVDTime",
                              collector=collector_managed_by_CAF,  # The other collectors are in the pre_collector_path itself
                              algorithms=list_of_algorithms,
                              input_files=good_input_files,
                              pre_collector_path=pre_collector_path)

    calibration.strategies = strategies.SequentialBoundaries

    for algorithm in calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    #########################################################
    # Add new fake calibration to run validation collectors #
    #########################################################

    val_cog6 = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog6_suffix}",
        clusters=f"SVDClusters{cog6_suffix}",
        time_algorithm="CoG6")

    val_cog6_onTracks = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog6_suffix}_onTracks",
        clusters=f"SVDClusters{cog6_suffix}_onTracks",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm="CoG6")

    val_cog3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog3_suffix}",
        clusters=f"SVDClusters{cog3_suffix}",
        time_algorithm="CoG3")

    val_cog3_onTracks = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog3_suffix}_onTracks",
        clusters=f"SVDClusters{cog3_suffix}_onTracks",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm="CoG3")

    val_els3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{els3_suffix}",
        clusters=f"SVDClusters{els3_suffix}",
        time_algorithm="ELS3")

    val_els3_onTracks = create_svd_clusterizer(
        name=f"ClusterReconstruction{els3_suffix}_onTracks",
        clusters=f"SVDClusters{els3_suffix}_onTracks",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm="ELS3")

    val_coll_cog6 = create_validation_collector(
        name=f"SVDTimeValidationCollector{cog6_suffix}",
        clusters=f"SVDClusters{cog6_suffix}",
        clusters_onTracks=f"SVDClusters{cog6_suffix}_onTracks",
        event_t0="EventT0")

    val_algo_cog6 = create_validation_algorithm(
        prefix=val_coll_cog6.name(),
        min_entries=10000)

    val_coll_cog3 = create_validation_collector(
        name=f"SVDTimeValidationCollector{cog3_suffix}",
        clusters=f"SVDClusters{cog3_suffix}",
        clusters_onTracks=f"SVDClusters{cog3_suffix}_onTracks",
        event_t0="EventT0")

    val_algo_cog3 = create_validation_algorithm(
        prefix=val_coll_cog3.name(),
        min_entries=10000)

    val_coll_els3 = create_validation_collector(
        name=f"SVDTimeValidationCollector{els3_suffix}",
        clusters=f"SVDClusters{els3_suffix}",
        clusters_onTracks=f"SVDClusters{els3_suffix}_onTracks",
        event_t0="EventT0")

    val_algo_els3 = create_validation_algorithm(
        prefix=val_coll_els3.name(),
        min_entries=10000)

    list_of_val_clusterizers = []
    list_of_val_algorithms = []
    for a in timeAlgorithms:
        if a == "CoG3":
            list_of_val_clusterizers.append(val_cog3)
            list_of_val_clusterizers.append(val_cog3_onTracks)
            list_of_val_algorithms.append(val_algo_cog3)
        if a == "CoG6":
            list_of_val_clusterizers.append(val_cog6)
            list_of_val_clusterizers.append(val_cog6_onTracks)
            list_of_val_algorithms.append(val_algo_cog6)
        if a == "ELS3":
            list_of_val_clusterizers.append(val_els3)
            list_of_val_clusterizers.append(val_els3_onTracks)
            list_of_val_algorithms.append(val_algo_els3)

    val_pre_collector_path = create_pre_collector_path(
        clusterizers=list_of_val_clusterizers,
        isMC=isMC, max_events_per_run=max_events_per_run,
        max_events_per_file=max_events_per_file,
        useSVDGrouping=useSVDGrouping, useRawtimeForTracking=useRawtimeForTracking,
        is_validation=True)

    val_collector_managed_by_CAF = val_coll_els3
    if "ELS3" in timeAlgorithms:
        val_collector_managed_by_CAF = val_coll_els3
        if "CoG6" in timeAlgorithms:
            val_pre_collector_path.add_module(val_coll_cog6)
        if "CoG3" in timeAlgorithms:
            val_pre_collector_path.add_module(val_coll_cog3)
    elif "CoG3" in timeAlgorithms:
        val_collector_managed_by_CAF = val_coll_cog3
        if "CoG6" in timeAlgorithms:
            val_pre_collector_path.add_module(val_coll_cog6)
    else:
        val_collector_managed_by_CAF = val_coll_cog6

    val_calibration = Calibration("SVDTimeValidation",
                                  collector=val_collector_managed_by_CAF,
                                  algorithms=list_of_val_algorithms,
                                  input_files=good_input_files,
                                  pre_collector_path=val_pre_collector_path)

    val_calibration.strategies = strategies.SequentialRunByRun

    for algorithm in val_calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    val_calibration.depends_on(calibration)

    return [calibration, val_calibration]
