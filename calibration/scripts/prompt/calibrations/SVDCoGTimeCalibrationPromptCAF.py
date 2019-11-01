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

import reconstruction as reco
import modularAnalysis as ana

input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'SVDShaperDigits',
]

now = datetime.datetime.now()
uniqueID = "SVDCoGTimeCalibrations_" + str(now.isoformat()) + "_INFO:_3rdOrderPol_TBindep_lat=+47.16"

settings = CalibrationSettings(name="SVDCoGTimeCalibrationPromptTest",
                               expert_username="ddossett",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["hlt_hadron"],
                               depends_on=[])

##################################################################
# Remove Module form the Path


def remove_module(path, name):

    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path
#####################################################################################################
# Definition of the class


def get_calibrations(input_data, **kwargs):

    # Set-up re-processing path
    path = create_path()

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)

    # run SVD reconstruction, changing names of StoreArray
    reco.add_svd_reconstruction(path)

    for moda in path.modules():
        if moda.name() == 'SVDCoGTimeEstimator':
            moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
        if moda.name() == 'SVDSimpleClusterizer':
            moda.param("Clusters", 'SVDClustersFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
        if moda.name() == 'SVDSpacePointCreator':
            moda.param("SVDClusters", 'SVDClustersFromTracks')

    path = remove_module(path, 'SVDMissingAPVsClusterCreator')

    file_to_iov_physics = input_data["hlt_hadron"]

    max_files_per_run = 1

    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")

    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # collector setup
    collector = register_module('SVDCoGTimeCalibrationCollector')
    collector.param("SVDClustersFromTracksName", "SVDClustersFromTracks")
    collector.param("SVDRecoDigitsFromTracksName", "SVDRecoDigitsFromTracks")
    collector.param("granularity", "run")

    # algorithm setup
    algorithm = SVDCoGTimeCalibrationAlgorithm(uniqueID)

    # calibration setup
    calibration = Calibration('SVDCoGTime',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=input_files_physics,
                              pre_collector_path=path,
                              )

    # calibration.pre_algorithms = pre_alg
    calibration.strategies = strategies.SequentialRunByRun
    # calibration.strategies = strategies.SingleIOV

    for algorithm in calibration.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [calibration]
