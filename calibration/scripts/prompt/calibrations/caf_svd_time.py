#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
Script to perform the svd time calibration with the CoG6, CoG3 and ELS3 algorithms
'''

import sys
import datetime
import random

from ROOT.Belle2 import SVDCoGTimeCalibrationAlgorithm
from ROOT.Belle2 import SVD3SampleCoGTimeCalibrationAlgorithm
from ROOT.Belle2 import SVD3SampleELSTimeCalibrationAlgorithm

import basf2 as b2

import rawdata as raw
from tracking import add_tracking_reconstruction

from caf.framework import Calibration
from caf import strategies
from caf.utils import IoV
from prompt import CalibrationSettings
from prompt.utils import filter_by_max_events_per_run

b2.set_log_level(b2.LogLevel.INFO)

random.seed(42)

now = datetime.datetime.now()
isMC = False

settings = CalibrationSettings(name="caf_svd_time",
                               expert_username="gdujany",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hlt_hadron"],
                               depends_on=[],
                               expert_config={
                                   "max_events_per_run": 10000,
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
                     granularity="run"):
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

    return collector


def create_algorithm(unique_id, prefix="", min_entries=10000):
    """
    Simply creates a SVDCoGTimeCalibrationAlgorithm class with some options.

    Returns:
        ROOT.Belle2.SVDCoGTimeCalibrationAlgorithm
    """
    if "CoG6" in prefix:
        algorithm = SVDCoGTimeCalibrationAlgorithm(unique_id)
    if "CoG3" in prefix:
        algorithm = SVD3SampleCoGTimeCalibrationAlgorithm(unique_id)
    if "ELS3" in prefix:
        algorithm = SVD3SampleELSTimeCalibrationAlgorithm(unique_id)
    if prefix:
        algorithm.setPrefix(prefix)
    algorithm.setMinEntries(10000)

    return algorithm


def create_svd_clusterizer(name="ClusterReconstruction",
                           clusters="SVDClustersFromTracks",
                           reco_digits=NEW_RECO_DIGITS_NAME,
                           shaper_digits=NEW_SHAPER_DIGITS_NAME,
                           time_algorithm=0):
    """
    Simply creates a SVDSimpleClusterizer module with some options.

    Returns:
        pybasf2.Module
    """

    cluster = b2.register_module("SVDSimpleClusterizer")
    cluster.set_name(name)
    cluster.param("Clusters", clusters)
    cluster.param("RecoDigits", reco_digits)
    cluster.param("ShaperDigits", shaper_digits)
    cluster.param("timeAlgorithm", time_algorithm)
    return cluster


def create_pre_collector_path(clusterizers):
    """
    Create a basf2 path that runs a common reconstruction path and also runs several SVDSimpleClusterizer
    modules with different configurations. This way they re-use the same reconstructed objects.

    Parameters:
        clusterizers (list[pybasf2.Module]): All the differently configured SVDSimpleClusterizer modules.
        They should output to different datastore objects.

    returns:
        pybasf2.Path
    """
    # Set-up re-processing path
    path = b2.create_path()

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

    # run tracking reconstruction
    add_tracking_reconstruction(path)
    path = remove_module(path, "V0Finder")
    b2.set_module_parameters(path, 'SVDGoGTimeEstimator', CalibrationWithEventT0=False)
    path.add_module("SVDShaperDigitsFromTracks")

    # repeat svd reconstruction using only SVDShaperDigitsFromTracks
    cog = b2.register_module("SVDCoGTimeEstimator")
    cog.set_name("CoGReconstruction")
    path.add_module(cog)

    # Debugging misconfigured Datastore names
    #    path.add_module("PrintCollections")

    for cluster in clusterizers:
        path.add_module(cluster)

    for moda in path.modules():
        if moda.name() == "CoGReconstruction":
            moda.param("ShaperDigits", NEW_SHAPER_DIGITS_NAME)
            moda.param("RecoDigits", NEW_RECO_DIGITS_NAME)
            moda.param("CalibrationWithEventT0", False)

    path = remove_module(path, "SVDMissingAPVsClusterCreator")

    return path


def get_calibrations(input_data, **kwargs):

    file_to_iov_physics = input_data["hlt_hadron"]
    expert_config = kwargs.get("expert_config")
    max_events_per_run = expert_config["max_events_per_run"]  # Maximum number of events selected per each run

    reduced_file_to_iov_physics = filter_by_max_events_per_run(file_to_iov_physics,
                                                               max_events_per_run, random_select=True)
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
        time_algorithm=0)

    cog3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog3_suffix}",
        clusters=f"SVDClustersFromTracks{cog3_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm=1)

    els3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{els3_suffix}",
        clusters=f"SVDClustersFromTracks{els3_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm=2)

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
        event_t0="EventT0")

    algo_cog6 = create_algorithm(
        unique_id_cog6,
        prefix=coll_cog6.name(),
        min_entries=10000)

    coll_cog3 = create_collector(
        name=f"SVDTimeCalibrationCollector{cog3_suffix}",
        clusters=f"SVDClustersFromTracks{cog3_suffix}",
        event_info=eventInfo,
        event_t0="EventT0")

    algo_cog3 = create_algorithm(
        unique_id_cog3,
        prefix=coll_cog3.name(),
        min_entries=10000)

    coll_els3 = create_collector(
        name=f"SVDTimeCalibrationCollector{els3_suffix}",
        clusters=f"SVDClustersFromTracks{els3_suffix}",
        event_info=eventInfo,
        event_t0="EventT0")

    algo_els3 = create_algorithm(
        unique_id_els3,
        prefix=coll_els3.name(),
        min_entries=10000)

    ####
    # Build the pre_collector_path for reconstruction BUT we also sneakily
    # add the two cog collectors to it.
    ####

    pre_collector_path = create_pre_collector_path(
        clusterizers=[cog6, cog3, els3])
    pre_collector_path.add_module(coll_cog6)
    pre_collector_path.add_module(coll_cog3)
    # We leave the coll_els3 to be the one "managed" by the CAF

    # calibration setup
    calibration = Calibration("SVDTime",
                              collector=coll_els3,   # The other collectors are in the pre_collector_path itself
                              algorithms=[algo_cog6, algo_cog3, algo_els3],
                              input_files=good_input_files,
                              pre_collector_path=pre_collector_path)

    # calibration.strategies = strategies.SequentialRunByRun
    # calibration.strategies = strategies.SingleIOV
    calibration.strategies = strategies.SequentialBoundaries

    for algorithm in calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    return [calibration]
