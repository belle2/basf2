from basf2 import *

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
from caf.strategies import SequentialBoundaries
import svd as svd

set_log_level(LogLevel.INFO)
input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'RawSVDs'
    # 'SVDShaperDigits',
    # 'SVDEventInfo'
]

now = datetime.datetime.now()


def remove_module(path, name):

    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path

# pre_collector


def pre_collector():

    B2INFO("Pre-collector")
    pre_path = create_path()
    pre_path.add_module('RootInput', branchNames=input_branches)

    pre_path.add_module("Gearbox")
    pre_path.add_module("Geometry", useDB=True)

    # run SVD unpacker
    svd.add_svd_unpacker(pre_path)

    # run SVD reconstruction, changing names of StoreArray
    reco.add_svd_reconstruction(pre_path)

    for moda in pre_path.modules():
        if moda.name() == 'SVDCoGTimeEstimator':
            moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
            moda.param("CalibrationWithEventT0", False)
        if moda.name() == 'SVDSimpleClusterizer':
            moda.param("Clusters", 'SVDClustersFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
            moda.param("timeAlgorithm", 0)
        if moda.name() == 'SVDSpacePointCreator':
            moda.param("SVDClusters", 'SVDClustersFromTracks')

    pre_path = remove_module(pre_path, 'SVDMissingAPVsClusterCreator')

    print_path(pre_path)

    return pre_path


def SVDCoGTimeCalibration(files, tags, uniqueID):

    # Set-up re-processing path
    path = create_path()

    path.add_module('Progress')

    # collector setup
    collector = register_module('SVDCoGTimeCalibrationCollector')
    collector.param("SVDClustersFromTracksName", "SVDClustersFromTracks")
    collector.param("SVDEventInfoName", "SVDEventInfo")
    collector.param("EventT0Name", "EventT0")
    collector.param("granularity", "run")

    # algorithm setup
    algorithm = SVDCoGTimeCalibrationAlgorithm(uniqueID)
    algorithm.setMinEntries(100)
    algorithm.setAllowedTimeShift(2.)

    # calibration setup
    calibration = Calibration('SVDCoGTime',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=files,
                              pre_collector_path=pre_collector(),
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )

    calibration.strategies = strategies.SequentialBoundaries

    return calibration


if __name__ == "__main__":

    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    print(" ")
    print("INPUT FILES")
    print(" ")
    print(input_files)
    print(" ")

    good_input_files = []
    runs = []
    expNum = int()
    for i in input_files:
        file_list = glob.glob(i)
        for f in file_list:
            tf = TFile.Open(f)
            tree = tf.Get("tree")
            if tree.GetEntries() != 0:
                good_input_files.append(f)
                print(str(f))
                inputStringSplit = f.split("/")
                for string in inputStringSplit:
                    if string.startswith('r0'):
                        s_run = str(string)
                        runNum = runs.append(int(s_run[1:6]))
                    if string.startswith('e0'):
                        s_exp = str(string)
                        expNum = int(s_exp[1:5])

    runs.sort()

    firstRun = runs[0]
    lastRun = runs[-1]

    if not len(good_input_files):
        print("You have to specify some input file(s)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

    # print(good_input_files)

    uniqueID = "SVDCoGTimeCalibrations_" + str(now.isoformat()) + "_INFO:_3rdOrderPol_TBindep_Exp" + \
        str(expNum) + "_runsFrom" + str(firstRun) + "to" + str(lastRun)
    print("")
    print("UniqueID")
    print("")
    print(str(uniqueID))
    print("")
    conditions.override_globaltags()
    svdCoGCAF = SVDCoGTimeCalibration(good_input_files,
                                      ["online_proc11",
                                       "data_reprocessing_proc11_baseline",
                                       "staging_data_reprocessing_proc11",
                                       "svd_NOCoGCorrections"],
                                      uniqueID)

    cal_fw = CAF()
    cal_fw.add_calibration(svdCoGCAF)
    cal_fw.backend = backends.LSF()
    cal_fw.run()
