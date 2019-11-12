from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import BeamSpotAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana
import vertex as vx

# Not needed for di-muon skim cdst or mdst, but needed to re-run reconstruction
input_branches = [
    'EventMetaData',
    'RawTRGs',
    'RawFTSWs',
    'RawPXDs',
    'RawSVDs',
    'RawCDCs']


def BeamSpotCalibration(files, tags):

    # Set-up re-processing path
    path = create_path()

    path.add_module('Progress')

    path.add_module('RootInput')
    # Not needed for di-muon skim cdst or mdst, but needed to re-run reconstruction
    # Remove all non-raw data to run the full reco again
    #    path.add_module('RootInput', branchNames=input_branches)

    # Not needed for di-muon skim cdst or mdst, but needed to re-run reconstruction
    # with possibly changed global tags
    #    raw.add_unpackers(path, components=["PXD", "SVD", "CDC"])
    #    reco.add_tracking_reconstruction(path)

    muSelection = 'p>1.0'
    muSelection += ' and abs(dz)<2.0 and dr<0.5'
    muSelection += ' and nPXDHits >=1 and nSVDHits >= 8 and nCDCHits >= 20'
    ana.fillParticleList('mu+:BS', muSelection, path=path)
    ana.reconstructDecay('Upsilon(4S):BS -> mu+:BS mu-:BS', '9.5<M<11.5', path=path)
    vx.vertexKFit('Upsilon(4S):BS', conf_level=0, path=path)

    collector = register_module('BeamSpotCollector', Y4SPListName='Upsilon(4S):BS')
    algorithm = BeamSpotAlgorithm()

    calibration = Calibration('BeamSpot',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )

    calibration.strategies = strategies.SequentialRunByRun

    return calibration


if __name__ == "__main__":
    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    if not len(input_files):
        print("You have to specify some input file(s) (raw data or di - muon skim with raw objects)\n"
              "using the standard basf2 command line option - i")
        print("See: basf2 -h")
        sys.exit(1)

    beamspot = BeamSpotCalibration(input_files, ['data_reprocessing_prompt_rel4_patchb', 'data_reprocessing_proc10'])
    # beamspot.max_iterations = 0

    cal_fw = CAF()
    cal_fw.add_calibration(beamspot)
    cal_fw.backend = backends.LSF()

    # Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(8)

    cal_fw.run()
