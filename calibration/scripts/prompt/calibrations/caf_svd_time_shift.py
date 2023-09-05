#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
Script to perform the svd time shift calibration with the CoG6, CoG3 and ELS3 algorithms
'''

import sys
import datetime
import random

from ROOT.Belle2 import SVDClusterTimeShifterAlgorithm
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

settings = CalibrationSettings(name="caf_svd_time_shift",
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
                                   "timeAlgorithms": ["CoG3", "ELS3", "CoG6"],
                                   "max_events_per_run":  60000,
                                   "max_events_per_file": 30000,
                                   "isMC": False,
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


NEW_SHAPER_DIGITS_NAME = "SVDShaperDigitsFromTracks"


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
                           shaper_digits=None,
                           time_algorithm="CoG6",
                           shiftSVDClusterTime=True):
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
    cluster.param("shiftSVDClusterTime", shiftSVDClusterTime)
    cluster.param("useDB", False)
    return cluster


def create_pre_collector_path(
        clusterizers,
        isMC=False,
        max_events_per_run=10000,
        max_events_per_file=10000,
        useSVDGrouping=True,
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
                b2.set_module_parameters(path, 'SVDTimeGrouping',
                                         forceGroupingFromDB=False,
                                         useParamFromDB=False,
                                         isEnabledIn6Samples=True, acceptSigmaN=3,
                                         expectedSignalTimeMin=-30, expectedSignalTimeMax=30)
                b2.set_module_parameters(path, 'SVDSpacePointCreator',
                                         forceGroupingFromDB=False,
                                         useSVDGroupInfoIn6Sample=True)
            else:
                b2.set_module_parameters(path, 'SVDTimeGrouping', forceGroupingFromDB=False, isEnabledIn6Samples=False)
                b2.set_module_parameters(path, 'SVDSpacePointCreator', forceGroupingFromDB=False, useSVDGroupInfoIn6Sample=False)

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
    useSVDGrouping = expert_config["useSVDGrouping"]

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

    #########################################################
    # SVD Cluster Time Shifter                              #
    #########################################################

    SVDClustersOnTrackPrefix = "SVDClustersOnTrack"

    shift_clusterizers_onTracks = []
    for alg in timeAlgorithms:
        cluster = create_svd_clusterizer(name=f"ClusterReconstruction_{alg}",
                                         clusters=f"{SVDClustersOnTrackPrefix}_{alg}",
                                         shaper_digits=NEW_SHAPER_DIGITS_NAME,
                                         time_algorithm=alg,
                                         shiftSVDClusterTime=False)
        shift_clusterizers_onTracks.append(cluster)

    shift_pre_collector_path = create_pre_collector_path(
        clusterizers=shift_clusterizers_onTracks,
        isMC=isMC, max_events_per_run=max_events_per_run,
        useSVDGrouping=useSVDGrouping)

    shift_collector = b2.register_module("SVDClusterTimeShifterCollector")
    shift_collector.set_name("SVDClusterTimeShifterCollector")
    shift_collector.param("MaxClusterSize", 6)
    shift_collector.param("EventT0Name", "EventT0")
    shift_collector.param("SVDClustersOnTrackPrefix", f"{SVDClustersOnTrackPrefix}")
    shift_collector.param("TimeAlgorithms", timeAlgorithms)

    shift_algo = SVDClusterTimeShifterAlgorithm(f"{calType}_{now.isoformat()}_INFO:_"
                                                f"Exp{expNum}_runsFrom{firstRun}to{lastRun}")
    shift_algo.setMinEntries(100)
    shift_algo.setMaximumAllowedShift(15.)
    shift_algo.setTimeAlgorithm(timeAlgorithms)

    shift_calibration = Calibration("SVDClusterTimeShifter",
                                    collector=shift_collector,
                                    algorithms=shift_algo,
                                    input_files=good_input_files,
                                    pre_collector_path=shift_pre_collector_path)

    shift_calibration.strategies = strategies.SingleIOV

    for algorithm in shift_calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    #########################################################
    # Add new fake calibration to run validation collectors #
    #########################################################

    list_of_val_clusterizers = []
    list_of_val_collectors = []
    list_of_val_algorithms = []
    for alg in timeAlgorithms:
        val_cluster = create_svd_clusterizer(
            name=f"ClusterReconstruction_{alg}",
            clusters=f"SVDClusters_{alg}",
            time_algorithm=alg)
        val_cluster_onTracks = create_svd_clusterizer(
            name=f"ClusterReconstruction_{alg}_onTracks",
            clusters=f"SVDClusters_{alg}_onTracks",
            shaper_digits=NEW_SHAPER_DIGITS_NAME,
            time_algorithm=alg)
        val_coll = create_validation_collector(
            name=f"SVDTimeValidationCollector_{alg}",
            clusters=f"SVDClusters_{alg}",
            clusters_onTracks=f"SVDClusters_{alg}_onTracks",
            event_t0="EventT0")
        val_algo = create_validation_algorithm(
            prefix=val_coll.name(),
            min_entries=10000)

        list_of_val_clusterizers.append(val_cluster)
        list_of_val_clusterizers.append(val_cluster_onTracks)
        list_of_val_collectors.append(val_coll)
        list_of_val_algorithms.append(val_algo)

    val_pre_collector_path = create_pre_collector_path(
        clusterizers=list_of_val_clusterizers,
        isMC=isMC, max_events_per_run=max_events_per_run,
        max_events_per_file=max_events_per_file,
        useSVDGrouping=useSVDGrouping,
        is_validation=True)

    val_collector_managed_by_CAF = list_of_val_collectors[0]
    for val_coll in list_of_val_collectors[1:]:
        val_pre_collector_path.add_module(val_coll)

    val_calibration = Calibration("SVDTimeValidation",
                                  collector=val_collector_managed_by_CAF,
                                  algorithms=list_of_val_algorithms,
                                  input_files=good_input_files,
                                  pre_collector_path=val_pre_collector_path)

    val_calibration.strategies = strategies.SequentialRunByRun

    for algorithm in val_calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    val_calibration.depends_on(shift_calibration)

    return [shift_calibration, val_calibration]
