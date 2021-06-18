import basf2 as b2

import os
import sys
import datetime
import glob

from ROOT import Belle2, TFile
from ROOT.Belle2 import SVD3SampleCoGTimeCalibrationAlgorithm  # changed

from caf.framework import CAF, Calibration, CentralDatabase
from caf import backends
from caf import strategies

import reconstruction as reco
import svd as svd

b2.set_log_level(b2.LogLevel.INFO)
input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'RawSVDs'
    # 'SVDShaperDigits',
    # 'SVDEventInfo'
]

now = datetime.datetime.now()


# pre_collector


def pre_collector():

    b2.B2INFO("Pre-collector")
    pre_path = b2.create_path()
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
        if moda.name() == 'SVDSimpleClusterizer':
            moda.param("Clusters", 'SVDClustersFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
            moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')
            moda.param("timeAlgorithm", 1)  # changed
        if moda.name() == 'SVDSpacePointCreator':
            moda.param("SVDClusters", 'SVDClustersFromTracks')

    pre_path = b2.remove_module(pre_path, 'SVDMissingAPVsClusterCreator')

    b2.print_path(pre_path)

    return pre_path


def SVDCoGTimeCalibration(files, tags, uniqueID):

    # Set-up re-processing path
    path = b2.create_path()

    path.add_module('Progress')

    # collector setup
    collector = b2.register_module('SVDTimeCalibrationCollector')
    collector.param("SVDClustersFromTracksName", "SVDClustersFromTracks")
    collector.param("SVDEventInfoName", "SVDEventInfo")
    collector.param("EventT0Name", "EventT0")
    collector.param("granularity", "run")
    collector.param("RawCoGBinWidth", 0.5)

    # algorithm setup
    algorithm = SVD3SampleCoGTimeCalibrationAlgorithm(uniqueID)  # changed
    algorithm.setMinEntries(100)
    algorithm.setAllowedTimeShift(2.)

    # calibration setup
    calibration = Calibration('SVD3SampleCoGTime',  # changed
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

    uniqueID = "SVD3SampleCoGTimeCalibrations_" + str(now.isoformat()) + "_INFO:_3rdOrderPol_TBindep_Exp" + \
        str(expNum) + "_runsFrom" + str(firstRun) + "to" + str(lastRun)  # changed
    print("")
    print("UniqueID")
    print("")
    print(str(uniqueID))
    print("")
    b2.conditions.override_globaltags()
    svdCoGCAF = SVDCoGTimeCalibration(good_input_files,
                                      [  # "online_proc11",
                                          "online", "Reco_master_patch_rel5"],
                                      # "data_reprocessing_proc11_baseline",
                                      # "staging_data_reprocessing_proc11",
                                      # "data_reprocessing_proc10",
                                      # "svd_NOCoGCorrections"],
                                      uniqueID)

    cal_fw = CAF()
    cal_fw.add_calibration(svdCoGCAF)
    cal_fw.backend = backends.LSF()
    cal_fw.run()
