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

import alignment.parameters
import alignment.constraints


def define_timedep_bucket7():
    # input_files_physics = ['/group/belle2/dataprod/Data/Raw/e0008/r031*/sub00/physics.*.root']
    #                                       14M   1.5M  277k  33k   1M    800k  8M
    physics_runs = [(0, run, 8) for run in [3115, 3118, 3119, 3120, 3121, 3122, 3123]]

    Mevt = int(1e6)
    each_1Mth_physics_event = [(ev, run, exp) for ev in range(0, 10*Mevt, Mevt) for (_, run, exp) in physics_runs]

    """
  b2conditionsdb iov data_reprocessing_prompt_rel4_patch -f MagneticField
  ------------------------------------------------------------------------------------------------------------------------------
  Name                                                                          Rev First  First  Final  Final  IovId  PayloadId
                                                                                    Exp    Run    Exp    Run
  ------------------------------------------------------------------------------------------------------------------------------
  (...)
  MagneticField                                                                 8   7      2811   8      1916   795145 71836  ON
  MagneticField                                                                 8   8      1917   8      2295   795143 71836  ON
  MagneticField                                                                 9   8      2296   8      2306   795220 74210  QCSOFF
  MagneticField                                                                 8   8      2307   8      3462   795144 71836  ON
  MagneticField                                                                 21  8      3463   -1     -1     800979 102314 OFF

  """

    # List of runs where magnetic field changed in bucket7
    Bfield_change_runs = [
      (0, 2811, 7),
      (0, 1917, 8),
      (0, 2296, 8),
      (0, 2307, 8),
      (0, 3463, 8)]

    timedep = [(alignment.parameters.cdc_layers(),
                Bfield_change_runs),
               (alignment.parameters.vxd_halfshells(),
                physics_runs + Bfield_change_runs),
               (alignment.parameters.vxd_sensors(layers=[1, 2], surface=False, rigid=True),
                physics_runs),
               (alignment.parameters.beamspot(),
                physics_runs)]

    return timedep


consts = [
  alignment.constraints.VXDHierarchyConstraints(type=2, svd=False),
  alignment.constraints.CDCLayerConstraints("myCDCLayerConstr.txt")]

timedep = define_timedep_bucket7()

fix_params = [alignment.parameters.vxd_ladders() +
              alignment.parameters.vxd_sensors(layers=[3, 4, 5, 6]) +
              alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=True)]


db_components = ['VXDAlignment', 'CDCAlignment']
components = ['PXD', 'SVD', 'CDC']


def VXDCDCalignment(files, tags):

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
                                timedepConfig=timedep,
                                enableSVDHierarchy=False
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
    algorithm.steering().command('entries 2 2')
    algorithm.steering().command('printcounts 2')
    # algorithm.steering().command('histprint')
    algorithm.steering().command('monitorresiduals')
    algorithm.steering().command('closeandreopen')
    algorithm.steering().command('hugecut 50.')
    # algorithm.steering().command('chiscut 30. 6.')
    # algorithm.steering().command('outlierdownweighting 3')
    # algorithm.steering().command('dwfractioncut 0.1')
    # algorithm.steering().command('presigmas 1.')
    algorithm.steering().command('FortranFiles')
    for filename in alignment.constraints.gen_constraints(consts, timedep, tags):
        algorithm.steering().command(filename)

    algorithm.steering().command('Parameters')

    def fix(labels):
        for label in labels:
            algorithm.steering().command('{} 0.0 -1.'.format(str(label)))

    # fix(alignment.parameters.vxd_halfshells(pxd=False, svd=True))
    fix(fix_params)

    algorithm.setTimedepConfig(timedep)

    calibration = Calibration('VXDCDCalignment',
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


if __name__ == "__main__":
    input_files = ['/group/belle2/dataprod/Data/Raw/e0008/r031*/sub00/physics.*.root']

    #                                       14M   1.5M  277k  33k   1M    800k  8M
    physics_runs = [(0, run, 8) for run in [3115, 3118, 3119, 3120, 3121, 3122, 3123]]

    input_files_cosmics = ['/group/belle2/dataprod/Data/Raw/e0008/r0{}/sub00/cosmic.*.root'.format(
        run) for run in [3463, 3464, 3466, 3467, 3468, 3469, 3470, 3471, 3472, 3475, 3476, 3478, 3479, 3480]]
    input_files_Bcosmics = ['/group/belle2/dataprod/Data/Raw/e0008/r0305*/sub00/cosmic.*.root']

    if len(Belle2.Environment.Instance().getInputFilesOverride()):
        input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    tags = conditions.default_globaltags
    tags = ['data_reprocessing_proc9', 'data_reprocessing_prompt_rel4_patch']

    mp2_beamspot = VXDCDCalignment(input_files, tags)
    mp2_beamspot.max_iterations = 1

    print("Starting CAF...")
    cal_fw = CAF()
    cal_fw.add_calibration(mp2_beamspot)
    cal_fw.backend = backends.LSF()

    # Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(1)

    # do some other stuff in the main process
    cal_fw.run()
