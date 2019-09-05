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

input_branches = [
    'EventMetaData',
    'RawTRGs',
    'RawFTSWs',
    'RawPXDs',
    'RawSVDs',
    'RawCDCs',
    'RawTOPs',
    'RawARICHs',
    'RawECLs',
    'RawKLMs']  # , 'SVDEventInfo']


def define_timedep_bucket7():
    # input_files_physics = ['/group/belle2/dataprod/Data/Raw/e0008/r031*/sub00/physics.*.root']
    #                                       14M   1.5M  277k  33k   1M    800k  8M
    physics_runs = [(0, run, 8) for run in [3115, 3118, 3119, 3120, 3121, 3122, 3123]]

    Mevt = int(1e6)
    each_1Mth_physics_event = [(ev, run, exp) for ev in range(0, 10*Mevt, Mevt) for (_, run, exp) in physics_runs]

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
               # (alignment.parameters.vxd_sensors(layers=[1, 2], surface=False, rigid=True),
               # physics_runs),
               (alignment.parameters.beamspot(),
                physics_runs)]

    return timedep


consts = [
  alignment.constraints.VXDHierarchyConstraints(type=2, svd=True, pxd=True),
  alignment.constraints.CDCLayerConstraints("myCDCLayerConstr.txt", z_offset=False)]

timedep = define_timedep_bucket7()

fix_params = [alignment.parameters.vxd_ladders() +
              # alignment.parameters.vxd_sensors(layers=[3, 4, 5, 6]) +
              alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=True)]


db_components = ['VXDAlignment', 'CDCAlignment']
components = ['PXD', 'SVD', 'CDC']


def Path_Cosmics():
    path = create_path()
    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput', branchNames=input_branches, entrySequences=['0:5000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')
    import rawdata as raw
    raw.add_unpackers(path)
    path.add_module('SVDEventInfoSetter')
    path.add_module('SetupGenfitExtrapolation')
    reco.add_cosmics_reconstruction(
        path,
        pruneTracks=False,
        skipGeometryAdding=True,
        addClusterExpertModules=False,
        data_taking_period='early_phase3',
        merge_tracks=True
    )

    tmp = create_path()
    for m in path.modules():
        if m.name() == 'EventsOfDoomBuster':  # or m.name()=='SVDCoGTimeEstimator':
            continue
        tmp.add_module(m)
    path = tmp
    tmp = create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            continue

        if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
            m.set_log_level(LogLevel.ERROR)

        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)

        tmp.add_module(m)

    path.add_module('SetRecoTrackMomentum')
    path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])

    return path


def VXDCDCalignment(files, tags, filesCosmicsB, filesCosmicsB0):

        # Set-up re-processing path
    path = create_path()

    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput', branchNames=input_branches, entrySequences=['0:5000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')
    # path.add_module('SVDEventInfoSetter')
    import rawdata as raw
    raw.add_unpackers(path)
    path.add_module('SVDEventInfoSetter')
    # Not needed for di-muon skim cdst or mdst, but needed to re-run reconstruction
    # with possibly changed global tags
    # raw.add_unpackers(path)

    reco.add_reconstruction(path, pruneTracks=False, components=components)
    tmp = create_path()
    for m in path.modules():
        if m.name() == 'EventsOfDoomBuster':  # or m.name()=='SVDCoGTimeEstimator':
            continue
        tmp.add_module(m)
    path = tmp
    tmp = create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            continue

        if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
            m.set_log_level(LogLevel.ERROR)

        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)

        tmp.add_module(m)

    path = tmp
    path.add_module('DAFRecoFitter')

    collector = register_module('MillepedeCollector',
                                tracks=['RecoTracks'],
                                primaryVertices=[],
                                calibrateVertex=True, components=db_components,
                                granularity='all',  # time dependence needs granularity=all
                                useGblTree=False,
                                absFilePaths=True,
                                minUsedCDCHitFraction=0.8,
                                minPValue=1.0e-5,
                                externalIterations=0,
                                timedepConfig=timedep
                                )

    algorithm = MillepedeAlgorithm()
    algorithm.invertSign(True)
    algorithm.ignoreUndeterminedParams(True)

    std_components = ROOT.vector('string')()
    for component in db_components:
        std_components.push_back(component)
    algorithm.setComponents(std_components)

    algorithm.steering().command('method inversion 3 0.1')
    algorithm.steering().command('skipemptycons')
    algorithm.steering().command('threads 40 40')
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
    from alignment.constraints import generate_constraints
    for filename in generate_constraints(consts, timedep, tags):
        algorithm.steering().command(filename)

    algorithm.steering().command('Parameters')

    def fix(label_sets):
        for labels in label_sets:
            for label in labels:
                algorithm.steering().command('{} 0.0 -1.'.format(str(label)))
                print(label)

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

    collectionCosmicsB = Collection(collector=collector,
                                    input_files=filesCosmicsB,
                                    pre_collector_path=Path_Cosmics(),
                                    max_files_per_collector_job=1,
                                    database_chain=[CentralDatabase(tag) for tag in tags])
    calibration.add_collection('cosmicsB', collectionCosmicsB)

    collectionCosmicsB0 = Collection(collector=collector,
                                     input_files=filesCosmicsB0,
                                     pre_collector_path=Path_Cosmics(),
                                     max_files_per_collector_job=1,
                                     database_chain=[CentralDatabase(tag) for tag in tags])
    calibration.add_collection('cosmicsB0', collectionCosmicsB0)

    calibration.strategies = strategies.SingleIOV

    return calibration


if __name__ == "__main__":
    input_files = ['/group/belle2/dataprod/Data/Raw/e0008/r031*/sub00/physics.*.root']

    #                                       14M   1.5M  277k  33k   1M    800k  8M
    physics_runs = [(0, run, 8) for run in [3115, 3118, 3119, 3120, 3121, 3122, 3123]]

    input_files_cosmics = ['/group/belle2/dataprod/Data/Raw/e0008/r0{}/sub00/cosmic.*.root'.format(
        run) for run in [3463, 3464, 3466]]  # , 3467, 3468, 3469, 3470, 3471, 3472, 3475, 3476, 3478, 3479, 3480]]
    input_files_Bcosmics = ['/group/belle2/dataprod/Data/Raw/e0008/r0305*/sub00/cosmic.*.root']

    if len(Belle2.Environment.Instance().getInputFilesOverride()):
        input_files = [os.path.abspath(file) for file in Belle2.Environment.Instance().getInputFilesOverride()]

    tags = conditions.default_globaltags
    tags = ['data_reprocessing_proc9', 'data_reprocessing_prompt_bucket7',  'data_reprocessing_prompt_rel4_patch']

    mp2_beamspot = VXDCDCalignment(input_files, tags, filesCosmicsB=input_files_Bcosmics, filesCosmicsB0=input_files_cosmics)
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
