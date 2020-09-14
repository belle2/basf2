import basf2
from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing
import datetime
import glob

import ROOT
from ROOT import Belle2, TFile
from ROOT.Belle2 import SVDCoGTimeCalibrationAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from prompt import CalibrationSettings
from caf import backends
from caf import strategies
from caf.utils import ExpRun, IoV

import svd as svd
import modularAnalysis as ana
from caf.strategies import SequentialBoundaries
import rawdata as raw
import reconstruction as reconstruction

from tracking import add_tracking_reconstruction

from softwaretrigger.path_utils import (
 add_filter_software_trigger,
 add_skim_software_trigger
 )

now = datetime.datetime.now()

settings = CalibrationSettings(name="SVDCoGTimeCalibrationPrompt",
                               expert_username="gdujany",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hlt_hadron"],
                               depends_on=[])

##################################################################
# Remove Module from the Path


def remove_module(path, name):

    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path
#####################################################################################################
# Definition of the class


def pre_collector():

    # Set-up re-processing path
    path = create_path()

    # unpack raw svd data and produce: SVDEventInfo and SVDShaperDigits
    raw.add_unpackers(path)

    # run SVD reconstruction, changing names of StoreArray
    add_tracking_reconstruction(path)

    for moda in path.modules():
        if moda.name() == 'SVDCoGTimeEstimator':
            moda.param("CalibrationWithEventT0", False)

    path.add_module('SVDShaperDigitsFromTracks')

    cog = register_module("SVDCoGTimeEstimator")
    cog.set_name("CoGReconstruction")
    path.add_module(cog)

    cluster = register_module("SVDSimpleClusterizer")
    cluster.set_name("ClusterReconstruction")
    path.add_module(cluster)

    sp = register_module("SVDSpacePointCreator")
    sp.set_name("SPCreator")
    path.add_module(sp)

    for moda in path.modules():
        if moda.name() == 'CoGReconstruction':
            moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
            moda.param("CalibrationWithEventT0", False)
        if moda.name() == 'ClusterReconstruction':
            moda.param("Clusters", 'SVDClustersFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
            moda.param("timeAlgorithm", 0)
        if moda.name() == 'SPCreator':
            moda.param("SVDClusters", 'SVDClustersFromTracks')
            moda.param("SpacePoints", 'SPacePointsFromTracks')

    path = remove_module(path, 'SVDMissingAPVsClusterCreator')


def get_calibrations(input_data, **kwargs):

    file_to_iov_physics = input_data["hlt_hadron"]

    max_files_per_run = 1

    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run, min_events_per_file=1)
    good_input_files = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(good_input_files)}")

    exps = [i.exp_low for i in reduced_file_to_iov_physics.values()]
    runs = sorted([i.run_low for i in reduced_file_to_iov_physics.values()])

    firstRun = runs[0]
    lastRun = runs[-1]
    expNum = exps[0]

    if not len(good_input_files):
        print("No good input files found! Check that the input files have entries != 0!")
        sys.exit(1)

    uniqueID = "SVDCoGTimeCalibrations_Prompt_" + str(now.isoformat()) + "_INFO:_3rdOrderPol_TBindep_Exp" + \
        str(expNum) + "_runsFrom" + str(firstRun) + "to" + str(lastRun)
    print("")
    print("UniqueID")
    print("")
    print(str(uniqueID))
    print("")

    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # collector setup
    collector = register_module('SVDCoGTimeCalibrationCollector')
    collector.param("SVDClustersFromTracksName", "SVDClustersFromTracks")
    collector.param("SVDEventInfoName", "SVDEventInfo")
    collector.param("EventT0Name", "EventT0")
    collector.param("granularity", "run")

    # algorithm setup
    algorithm = SVDCoGTimeCalibrationAlgorithm(uniqueID)
    algorithm.setMinEntries(10000)

    # calibration setup
    calibration = Calibration('SVDCoGTime',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=good_input_files,
                              pre_collector_path=pre_collector(),
                              )

    # calibration.pre_algorithms = pre_alg
    # calibration.strategies = strategies.SequentialRunByRun
    # calibration.strategies = strategies.SingleIOV
    calibration.strategies = strategies.SequentialBoundaries

    for algorithm in calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    return [calibration]
