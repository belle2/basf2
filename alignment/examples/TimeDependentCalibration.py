from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import MillepedeAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana
import vertex as vx

from simulation import add_simulation
from L1trigger import add_tsim

# Generate 4 runs, with run number 4, 5, 6, 7
runList = [4, 5, 6, 7]
# all runs in experiment 0 (default final Phase3 config)
expList = [0, 0, 0, 0]
# let's have runs 5 and 7 longer and let the PXD move each 20 events below
evtNumList = [20, 60, 20, 40]

# VXD iDs of the 2 PXD half shells
ying = Belle2.VxdID(1, 0, 0, 1)
yang = Belle2.VxdID(1, 0, 0, 2)

# Their rigid body parameter IDs
# x,y, z shift and alpha, beta, gamma rotation
params = [1, 2, 3, 4, 5, 6]

label = Belle2.GlobalLabel()

pxd_labels = []
for shell in [ying, yang]:
    for par in params:
        label.construct(Belle2.VXDAlignment.getGlobalUniqueID(), shell.getID(), par)
        pxd_labels.append(label.label())

# Configure the the time dependence for calibration
timedep = [(pxd_labels, [(0, run, 0) for run in runList] + [(20, 5, 0), (40, 5, 0)] + [(20, 7, 0)])]

db_components = ['VXDAlignment']
components = ['PXD', 'SVD', 'CDC']


def PXDHalfShellsAlignment(files, tags):

        # Set-up re-processing path
    path = create_path()

    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    # path.add_module('RootInput', branchNames=input_branches)#, entrySequences=['0:1000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')
    # Not needed for di-muon skim cdst or mdst, but needed to re-run reconstruction
    # with possibly changed global tags
    # raw.add_unpackers(path)
    reco.add_mc_reconstruction(path, pruneTracks=False, components=components)
    path.add_module('DAFRecoFitter')

    collector = register_module('MillepedeCollector',
                                tracks=['RecoTracks'],
                                primaryVertices=[],
                                calibrateVertex=True, components=db_components,
                                granularity='all',  # time dependence needs granularity=all
                                useGblTree=False,
                                absFilePaths=True,
                                timedepConfig=timedep
                                )

    algorithm = MillepedeAlgorithm()
    algorithm.invertSign(True)
    algorithm.ignoreUndeterminedParams(False)

    std_components = ROOT.vector('string')()
    for component in db_components:
        std_components.push_back(component)
    algorithm.setComponents(std_components)

    algorithm.steering().command('method diagonalization 3 0.1')
    algorithm.steering().command('skipemptycons')
    algorithm.steering().command('threads 4 4')
    algorithm.steering().command('matiter 1')
    algorithm.steering().command('scaleerrors 1. 1.')
    # algorithm.steering().command('entries 10')
    algorithm.steering().command('printcounts 1')
    # algorithm.steering().command('histprint')
    algorithm.steering().command('monitorresiduals')
    algorithm.steering().command('closeandreopen')
    algorithm.steering().command('hugecut 50.')
    # algorithm.steering().command('chiscut 30. 6.')
    # algorithm.steering().command('outlierdownweighting 3')
    # algorithm.steering().command('dwfractioncut 0.1')
    # algorithm.steering().command('presigmas 1.')

    algorithm.steering().command('Parameters')

    def fix_vxd_id(vxd_id, params=None):
        if params is None:
            params = [1, 2, 3, 4, 5, 6]  # rigid body parameters
        for ipar in params:
            label = Belle2.GlobalLabel()
            label.construct(Belle2.VXDAlignment.getGlobalUniqueID(), vxd_id.getID(), ipar)
            algorithm.steering().command('{} 0.0 -1. ! {}'.format(str(label.label()), str(vxd_id)))

    # Halfshells
    # fix_vxd_id(Belle2.VxdID(1, 0, 0, 1)) # ying
    # fix_vxd_id(Belle2.VxdID(1, 0, 0, 2)) # yang
    fix_vxd_id(Belle2.VxdID(3, 0, 0, 1))  # pat
    fix_vxd_id(Belle2.VxdID(3, 0, 0, 2))  # mat

    ladders = [8, 12, 7, 10, 12, 16]
    sensors = [2, 2, 2, 3, 4, 5]

    for layer in range(1, 7):
        for ladder in range(1, ladders[layer - 1] + 1):
            fix_vxd_id(Belle2.VxdID(layer, ladder, 0), params=[1, 2, 3, 4, 5, 6])
            for sensor in range(1, sensors[layer - 1] + 1):
                # sensors
                fix_vxd_id(
                    Belle2.VxdID(
                        layer,
                        ladder,
                        sensor),
                    params=[
                        1,
                        2,
                        3,
                        4,
                        5,
                        6,
                        31,
                        32,
                        33,
                        41,
                        42,
                        43,
                        44,
                        51,
                        52,
                        53,
                        54,
                        55,
                        20])
                pass

    algorithm.setTimedepConfig(timedep)

    calibration = Calibration('PXDHalfShellsAlignment',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=files,
                              pre_collector_path=path,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )

    calibration.strategies = strategies.SingleIOV

    return calibration


def generate_test_data(filename):
    kkgeninput = register_module('KKGenInput')
    kkgeninput.param('tauinputFile', Belle2.FileSystem.findFile('data/generators/kkmc/mu.input.dat'))
    kkgeninput.param('KKdefaultFile', Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'))
    kkgeninput.param('taudecaytableFile', '')
    kkgeninput.param('kkmcoutputfilename', 'kkmc_mumu.txt')

    main = create_path()

    main.add_module("EventInfoSetter", evtNumList=evtNumList, runList=runList, expList=expList)
    # add_beamparameters(main, "Y4S")
    main.add_module('Gearbox')
    main.add_module('Geometry')

    main.add_module(kkgeninput)

    add_simulation(main)
    add_tsim(main)

    main.add_module("RootOutput", outputFileName=filename)
    main.add_module("Progress")

    process(main)
    print(statistics)
    return os.path.abspath(filename)

if __name__ == "__main__":
    input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    if not len(input_files):
        outfile = Belle2.Environment.Instance().getOutputFileOverride()
        if not outfile:
            outfile = 'TimeDepCalibration_TestData.root'
        print("No input file provided. This will now generate test data into {} and run over it.".format(outfile))
        print("It will take couple of minutes.")
        print("You can set this file as input next time using -i option of basf2")

        input_files = [generate_test_data(outfile)]

    tags = [conditions.default_globaltags]
    mp2_beamspot = PXDHalfShellsAlignment(input_files, tags)
    mp2_beamspot.max_iterations = 1

    cal_fw = CAF()
    cal_fw.add_calibration(mp2_beamspot)
    cal_fw.backend = backends.LSF()

    # Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(1)

    cal_fw.run()
