from basf2 import *
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.WARNING)

import os
import sys
import multiprocessing
import datetime
import glob

import ROOT
from ROOT import Belle2, TFile
from ROOT.Belle2 import SVDCoGTimeCalibrationAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies
from caf.utils import ExpRun, IoV

import reconstruction as reco
import modularAnalysis as ana

input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'SVDShaperDigits'
]

now = datetime.datetime.now()
uniqueID = "SVDCoGTimeCalibrations_MC_" + str(now.isoformat()) + "_INFO:_3rdOrderPol_TBindep_lat=+47.16"


def remove_module(path, name):

    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path


def pre_alg(algorithm, iteration):

    B2INFO("Running pre_algorithm function")
    evtinfo = register_module('EventInfoSetter', evtNumList=[1], runList=[1], expList=[1003])
    evtinfo.initialize()
    gear = register_module('Gearbox')
    geom = register_module('Geometry')
    gear.initialize()
    geom.initialize()


def SVDCoGTimeCalibration(files, tags):

    # Set-up re-processing path
    path = create_path()

    path.add_module('Progress')

    # Remove not useful branches
    path.add_module('RootInput', branchNames=input_branches)

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

    # collector setup
    collector = register_module('SVDCoGTimeCalibrationCollector')
    collector.param("SVDClustersFromTracksName", "SVDClustersFromTracks")
    collector.param("SVDRecoDigitsFromTracksName", "SVDRecoDigitsFromTracks")
    collector.param("granularity", "run")

    algorithm = SVDCoGTimeCalibrationAlgorithm(uniqueID)

    # calibration setup
    calibration = Calibration('SVDCoGTime',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )

    calibration.pre_algorithms = pre_alg
    calibration.strategies = strategies.SequentialRunByRun
    # calibration.strategies = strategies.SingleIOV

    return calibration


if __name__ == "__main__":

    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    print(" ")
    print("INPUT FILES")
    print(" ")
    print(input_files)
    print(" ")

    good_input_files = []

    for i in input_files:
        file_list = glob.glob(i)
        for f in file_list:
            tf = TFile.Open(f)
            tree = tf.Get("tree")
            if tree.GetEntries() != 0:
                good_input_files.append(f)
                print(str(f))

    if not len(good_input_files):
        print("You have to specify some input file(s)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

    'data_reprocessing_prompt_rel4_patch'
    svdCoGCAF = SVDCoGTimeCalibration(good_input_files, ['data_reprocessing_prompt_rel4_patch', 'svd_NOCoGCorrections'])

    cal_fw = CAF()
    cal_fw.add_calibration(svdCoGCAF)
    cal_fw.backend = backends.LSF()

    # Try to guess if we are at KEKCC and change the backend to Local if not
#    if multiprocessing.cpu_count() < 10:
#        cal_fw.backend = backends.Local(8)

    # iov_to_calibrate = IoV(exp_low=8, run_low=0, exp_high=8, run_high=-1)
    # cal_fw.run(iov=iov_to_calibrate)
    cal_fw.run()
