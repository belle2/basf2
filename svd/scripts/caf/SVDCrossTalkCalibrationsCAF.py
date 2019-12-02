from basf2 import *

set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDCrossTalkCalibrationsAlgorithm
from ROOT.Belle2 import SVDOccupancyCalibrationsAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw

from basf2 import conditions

input_branches = [
    'RawSVDs'
]


def SVDCrossTalkCalibrations(files, tags):

    # Set-up re-processing path
    path = create_path()

    path.add_module('Progress')
    path.add_module('RootInput', branchNames=input_branches)

    path.add_module("Gearbox")
    path.add_module("Geometry")  # , useDB=True)

    raw.add_unpackers(path, components=['SVD'])

    collector = register_module('SVDCrossTalkCalibrationsCollector')
    algorithm = SVDCrossTalkCalibrationsAlgorithm("SVDCrossTalkCAF")

    calibration = Calibration('SVDCrossTalk',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=-1,
                              backend_args=None
                              )
    # Local database containing occupancy payload required for CrossTalk
    calibration.use_local_database('/gpfs/fs02/belle2/group/detector/SVD/earlyPhase3/localDB_Occupancy/database.txt')

    calibration.strategies = strategies.SequentialRunByRun

    return calibration


if __name__ == "__main__":
    input_files = [os.path.abspath(file) for file in [
            "/group/belle2/dataprod/Data/Raw/e0009/r00385/sub00/*root"]]

    svdCrossTalkCAF = SVDCrossTalkCalibrations(input_files,
                                               ['data_reprocessing_prompt_rel4_patchb',
                                                'svd_loadedOnFADC', 'svd_basic'])

    cal_fw = CAF()
    cal_fw.add_calibration(svdCrossTalkCAF)
    cal_fw.backend = backends.LSF()

    # Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(8)

    cal_fw.run()
