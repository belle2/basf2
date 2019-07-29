from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDCoGTimeCalibrationAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies

import reconstruction as reco
import modularAnalysis as ana

input_branches = [
    'SVDShaperDigitsFromTracks',
    'EventT0',
    'SVDShaperDigits'
]

# set_log_level(LogLevel.WARNING)


def SVDCoGTimeCalibration(files, tags):

    # Set-up re-processing path
    path = create_path()

    # logging.log_level = LogLevel.WARNING

    path.add_module('Progress')
    # Remove not useful branches
    path.add_module('RootInput', branchNames=input_branches)

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)

    # fil = register_module('SVDShaperDigitsFromTracks')
    # fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
    # main.add_module(fil)

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

    # collector setup
    collector = register_module('SVDCoGTimeCalibrationCollector')
    collector.param("SVDClustersFromTracksName", "SVDClustersFromTracks")
    collector.param("SVDRecoDigitsFromTracksName", "SVDRecoDigitsFromTracks")
    algorithm = SVDCoGTimeCalibrationAlgorithm("SVDCoGTimeCAF")

    # calibration setup
    calibration = Calibration('SVDCoGTime',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=-1,
                              backend_args=None
                              )

    calibration.strategies = strategies.SequentialRunByRun

    return calibration


if __name__ == "__main__":

    # input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]
    input_files = [os.path.abspath(file) for file in [
        "/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009/\
e0008/4S/r01309/skim/hlt_bhabha/cdst/sub00/cdst.physics.0008.01309.HLT*"]]

    if not len(input_files):
        print("You have to specify some input file(s)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

    svdCoGCAF = SVDCoGTimeCalibration(input_files, ['data_reprocessing_proc9'])

    cal_fw = CAF()
    cal_fw.add_calibration(svdCoGCAF)
    cal_fw.backend = backends.LSF()

    # Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(8)

    cal_fw.run()
