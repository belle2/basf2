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
import reconstruction as reco
import modularAnalysis as ana
# import vertex as vx

from basf2 import conditions


input_branches = [
    'RawSVDs'
]

# set_log_level(LogLevel.WARNING)


def SVDCrossTalkCalibrations(files, tags):

    # Set-up re-processing path
    path = create_path()

    # logging.log_level = LogLevel.WARNING

    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput', branchNames=input_branches)

    path.add_module("Gearbox")
    path.add_module("Geometry")  # , useDB=True)

    # fil = register_module('SVDShaperDigitsFromTracks')
    # fil.param('outputINArrayName', 'SVDShaperDigitsFromTracks')
    # main.add_module(fil)

    # Not needed for di-muon skim cdst or mdst, but needed to re-run reconstruction
    # with possibly changed global tags
    raw.add_unpackers(path, components=['SVD'])
    cog = register_module('SVDCoGTimeEstimator')
    path.add_module(cog)

    collector2 = register_module('SVDCrossTalkCalibrationsCollector')
#    collector2.param("SVDRecoDigits", "SVDRecoDigits")
    algorithm2 = SVDCrossTalkCalibrationsAlgorithm("SVDCrossTalkCAF")

    calibration = Calibration('SVDCrossTalk',
                              collector=collector2,
                              algorithms=algorithm2,
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              #                              use_local_database(),
                              output_patterns=None,
                              max_files_per_collector_job=-1,
                              backend_args=None
                              )
#    calibration.use_local_database('/gpfs/fs02/belle2/group/detector/SVD/earlyPhase3/localDB_Occupancy','/gpfs/fs02/belle2/group/detector/SVD/earlyPhase3/localDB_Occupancy/database.txt')
    calibration.use_local_database('/gpfs/fs02/belle2/group/detector/SVD/earlyPhase3/localDB_Occupancy/database.txt')

    calibration.strategies = strategies.SequentialRunByRun

    return calibration


if __name__ == "__main__":
    # use by default raw data from cdst of exp8, run1309 (shaperDigits need to be unpacked, not available in cdst format)
    input_files = [os.path.abspath(file) for file in [
            "/group/belle2/dataprod/Data/Raw/e0009/r00385/sub00/*root"]]
# "/group/belle2/dataprod/Data/release-03-02-02/DB00000635/proc00000009/\
# e0008/4S/r01309/skim/hlt_bhabha/cdst/sub00/cdst.physics.0008.01309.HLT*"]]

    if not len(input_files):
        print("You have to specify some input file(s)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

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
