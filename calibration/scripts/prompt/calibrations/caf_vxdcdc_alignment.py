# -*- coding: utf-8 -*-

"""

Full Simultaneous Global and Local VXD and CDC alignment with Millepede II

The input collections are:
- cosmics (hlt skim) - mandatorry
- hadron - for "low" momentum tracks from IP
- mumu - mumu_2trk or mumu_tight - for high momentum tracks from IP
- offip - tracks from outside IP (beam background, beam-gas)

"""

import basf2
import ROOT
from ROOT import Belle2

from prompt import CalibrationSettings, input_data_filters
from prompt.calibrations.caf_cdc import settings as cdc_calibration
from prompt.utils import events_in_basf2_file
from caf.utils import IoV
from caf import strategies

from softwaretrigger.constants import HLT_INPUT_OBJECTS
import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana
import vertex as vtx

from random import choice
from random import seed

import millepede_calibration as mpc
import alignment.collections
import alignment.constraints
import alignment.parameters

collection_names = ["cosmic", "hadron", "mumu", "offip"]

default_config = {
    "cosmic.max_processed_events_per_file": 4000,
    "hadron.max_processed_events_per_file": 1000,
    "mumu.max_processed_events_per_file": 5000,
    "offip.max_processed_events_per_file": 2000,
    "stage1.method": "fullLAPACK"
}

quality_flags = [input_data_filters["Run Type"]["physics"],
                 input_data_filters["Data Quality Tag"]["Good Or Recoverable"],
                 input_data_filters["Magnet"]["On"]]

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="Full VXD and CDC Alignment",
                               expert_username="bilkat",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=collection_names,
                               input_data_filters={
                                   "cosmic": [input_data_filters["Data Tag"]["cosmic_calib"]] + quality_flags,
                                   "mumu": [input_data_filters["Data Tag"]["mumutight_calib"]] + quality_flags,
                                   "hadron": [input_data_filters["Data Tag"]["hadron_calib"]] + quality_flags,
                                   "offip": [input_data_filters["Data Tag"]["offip_calib"]] + quality_flags
                               },

                               expert_config=default_config,
                               depends_on=[cdc_calibration])


def select_files(all_input_files, min_events, max_processed_events_per_file):
    """
    Select files randomly

    Parameters
    ----------
    all_input_files : list(str)
      List of all input file names
    min_events : int
      Minimum number of events to select from files
    max_processed_events_per_file : int
      Maximum number of events to consider per file
    """
    all_input_files = all_input_files[:]
    # Let's iterate, taking a sample of files from the total (no repeats or replacement) until we get enough events
    total_events = 0
    chosen_files = []
    while total_events < min_events:
        # If the set is empty we must have used all available files. Here we break and continue. But you may want to
        # raise an Error...
        if not all_input_files:
            break
        # Randomly select a file
        new_file_choice = choice(all_input_files)
        # Remove it from the list so it can't be chosen again
        all_input_files.remove(new_file_choice)
        # Find the number of events in the file
        total_events_in_file = events_in_basf2_file(new_file_choice)
        if not total_events_in_file:
            # Uh Oh! Zero event file, skip it
            continue

        events_contributed = min(total_events_in_file, max_processed_events_per_file)

        chosen_files.append(new_file_choice)
        total_events += events_contributed

    basf2.B2INFO(f"Total chosen files = {len(chosen_files)}")
    basf2.B2INFO(f"Total events in chosen files = {total_events}")
    if total_events < min_events:
        basf2.B2WARNING(
            f"There weren't enough files events selected when max_processed_events_per_file={max_processed_events_per_file}")
    return chosen_files


def create_std_path():
    """
    Returns default path for collections with standard reconstruction
    """
    path = basf2.create_path()
    path.add_module('Progress')
    path.add_module('RootInput')
    path.add_module('Gearbox')
    path.add_module('Geometry')
    raw.add_unpackers(path)
    path.add_module('SetupGenfitExtrapolation')
    reco.add_reconstruction(
        path,
        pruneTracks=False,
        skipGeometryAdding=True,
    )
    path.add_module('DAFRecoFitter')
    return path


def create_cosmics_path():
    """
    Returns default path for cosmic collection
    """
    path = basf2.create_path()
    path.add_module('Progress')
    path.add_module('RootInput')
    path.add_module('Gearbox')
    path.add_module('Geometry')

    raw.add_unpackers(path)
    path.add_module('SetupGenfitExtrapolation')
    reco.add_cosmics_reconstruction(
        path,
        pruneTracks=False,
        skipGeometryAdding=True,
        addClusterExpertModules=False,
        merge_tracks=True
    )

    path.add_module('SetRecoTrackMomentum', automatic=True)
    path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])

    ana.fillParticleList(
        'mu+:goodForVXDCDCAlignment',
        '[z0 <= 57. or abs(d0) >= 26.5] and abs(dz) > 0.4 and nTracks == 1',
        path=path)
    path.add_module('SkimFilter', particleLists=['mu+:goodForVXDCDCAlignment']).if_false(basf2.create_path())

    return path


def make_mumu_collection(
        name,
        files=None,
        muon_cut='p > 1.0 and abs(dz) < 2.0 and dr < 0.5 and nTracks==2',
        dimuon_cut='9.5 < M and M < 11.',
        prescale=1.):
    """
    Di-muons with vertex+beam constraint collection

    Parameters
    ----------
    name : str
      Collection name
    files : list(str)
      List of input data files
    muon_cut : str
      Cut string to select daughter muons
    dimuon_cut : str
      Cut string to apply for reconstructed di-muon decay
    prescale : float
      Process only 'prescale' fraction of events
    """
    path = basf2.create_path()
    path.add_module('Progress')
    path.add_module('RootInput')
    if prescale != 1.:
        path.add_module('Prescale', prescale=prescale).if_false(basf2.Path(), basf2.AfterConditionPath.END)

    path.add_module('Gearbox')
    path.add_module('Geometry')

    raw.add_unpackers(path)

    reco.add_reconstruction(path, pruneTracks=False)

    path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])

    ana.fillParticleList(f"mu+:{name}", muon_cut, path=path)
    ana.reconstructDecay(f"Upsilon(4S):{name} -> mu+:{name} mu-:{name}", dimuon_cut, path=path)

    vtx.raveFit(f"Upsilon(4S):{name}", 0.001, daughtersUpdate=True, silence_warning=True, path=path, constraint="ipprofile")

    return alignment.collections.make_collection(
        name,
        files=files,
        path=path,
        primaryVertices=[f"Upsilon(4S):{name}"])


def create_prompt(files, cfg):
    """
    Returns configured (original) prompt stage alignment

    Parameters
    ----------
    files : list(str)
      Dictionary with all input files by category (name)
    cfg : dict
      Expert config dictionary
    """
    mumu = select_files(files["mumu"], 0.2e6, cfg["mumu.max_processed_events_per_file"])
    cosmic = select_files(files["cosmic"], 1e6, cfg["cosmic.max_processed_events_per_file"])
    hadron = select_files(files["hadron"], 0.5e5, cfg["hadron.max_processed_events_per_file"])
    offip = select_files(files["offip"], 0.2e6, cfg["offip.max_processed_events_per_file"])

    cal = mpc.create(
        name='VXDCDCalignment_prompt',
        dbobjects=['VXDAlignment', 'CDCAlignment'],
        collections=[
            mpc.make_collection("cosmic", path=create_cosmics_path(), tracks=["RecoTracks"]),
            mpc.make_collection("hadron", path=create_std_path(), tracks=["RecoTracks"]),
            mpc.make_collection("mumu", path=create_std_path(), tracks=["RecoTracks"]),
            mpc.make_collection("offip", path=create_std_path(), tracks=["RecoTracks"])
        ],
        tags=None,
        files=dict(mumu=mumu, cosmic=cosmic, hadron=hadron, offip=offip),
        timedep=None,
        constraints=[
            alignment.constraints.VXDHierarchyConstraints(type=2, pxd=True, svd=True),
            alignment.constraints.CDCLayerConstraints(z_offset=True, z_scale=False, twist=True)
        ],
        fixed=alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=False),
        commands=[
            "method diagonalization 3 0.1",
            "scaleerrors 1. 1.",
            "entries 1000"],
        params=dict(minPValue=0.00001, externalIterations=0, granularity="run"),
        min_entries=1000000)

    cal.max_iterations = 5

    return cal


def create_beamspot(files, cfg):
    """
    Returns configured beamspot calibration

    Parameters
    ----------
    files : list(str)
      Dictionary with all input files by category (name)
    cfg : dict
      Expert config dictionary
    """

    mumu = files["mumu"]

    ###################################################
    # Algorithm setup

    from ROOT.Belle2 import BeamSpotAlgorithm
    from basf2 import create_path, register_module

    ###################################################
    # Calibration setup

    from caf.framework import Calibration, Collection
    from caf.strategies import SingleIOV

    # module to be run prior the collector
    path = create_path()

    path.add_module('Progress')
    path.add_module('RootInput')
    path.add_module('Gearbox')
    path.add_module('Geometry')
    raw.add_unpackers(path)
    path.add_module('SetupGenfitExtrapolation')
    reco.add_reconstruction(path, skipGeometryAdding=True)

    muSelection = '[p>1.0]'
    muSelection += ' and abs(dz)<2.0 and abs(dr)<0.5'
    muSelection += ' and nPXDHits >=1 and nSVDHits >= 8 and nCDCHits >= 20'
    ana.fillParticleList('mu+:BS', muSelection, path=path)
    ana.reconstructDecay('Upsilon(4S):BS -> mu+:BS mu-:BS', '9.5<M<11.5', path=path)

    collector_bs = register_module('BeamSpotCollector', Y4SPListName='Upsilon(4S):BS')
    algorithm_bs = BeamSpotAlgorithm()
    # algorithm_bs.setOuterLoss(cfg['outerLoss'])
    # algorithm_bs.setInnerLoss(cfg['innerLoss'])

    collection_bs = Collection(collector=collector_bs,
                               input_files=mumu,
                               pre_collector_path=path)

    calibration_bs = Calibration('VXDCDCalignment_beamspot', algorithms=algorithm_bs)
    calibration_bs.add_collection("mumu", collection_bs)

    calibration_bs.strategies = SingleIOV

    return calibration_bs


def create_stage1(files, cfg):
    """
    Returns configured stage1 alignment (full constant alignment with wires, beamspot fixed)

    Parameters
    ----------
    files : list(str)
      Dictionary with all input files by category (name)
    cfg : dict
      Expert config dictionary
    """

    mumu = select_files(files["mumu"], 1.5e6, cfg["mumu.max_processed_events_per_file"])
    cosmic = select_files(files["cosmic"], 0.7e6, cfg["cosmic.max_processed_events_per_file"])
    hadron_and_offip = select_files(files["hadron"] + files["offip"], int(4.0e6 / 10.), cfg["hadron.max_processed_events_per_file"])

    cal = mpc.create(
        name='VXDCDCalignment_stage1',
        dbobjects=['VXDAlignment', 'CDCAlignment', 'BeamSpot'],
        collections=[
            mpc.make_collection("cosmic", path=create_cosmics_path(), tracks=["RecoTracks"]),
            mpc.make_collection("hadron", path=create_std_path(), tracks=["RecoTracks"]),
            make_mumu_collection(name="mumu")
        ],
        tags=None,
        files=dict(mumu=mumu, cosmic=cosmic, hadron=hadron_and_offip),
        timedep=None,
        constraints=[
            alignment.constraints.VXDHierarchyConstraints(type=2, pxd=True, svd=True),
            alignment.constraints.CDCLayerConstraints(z_offset=False, z_scale=False, twist=False),
            alignment.constraints.CDCWireConstraints(layer_rigid=True, layer_radius=[53], cdc_radius=True, hemisphere=[55])
        ],
        fixed=alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=False)
        + alignment.parameters.beamspot(),
        commands=[
            f"method {cfg['stage1.method']} 6 0.001",
            "entries 1000",
            "threads 10 10"],
        params=dict(minPValue=0.00001, externalIterations=0, granularity="run"),
        min_entries=500000)

    # Ignore results for BeamSpot
    std_components = ROOT.vector('string')()
    for component in ['VXDAlignment', 'CDCAlignment']:
        std_components.push_back(component)
    cal.algorithms[0].algorithm.setComponents(std_components)

    cal.max_iterations = 0

    return cal


def create_stage2(files, cfg):
    """
    Returns configured stage2 alignment (run-dependent alignment)

    Parameters
    ----------
    files : list(str)
      Dictionary with all input files by category (name)
    cfg : dict
      Expert config dictionary
    """
    mumu = select_files(files["mumu"], 10e6, cfg["mumu.max_processed_events_per_file"])
    cosmic = select_files(files["cosmic"], 2e6, cfg["cosmic.max_processed_events_per_file"])

    cal = mpc.create(
        name='VXDCDCalignment_stage2',
        dbobjects=['VXDAlignment', 'CDCAlignment', 'BeamSpot'],
        collections=[
            mpc.make_collection("cosmic", path=create_cosmics_path(), tracks=["RecoTracks"]),
            make_mumu_collection(name="mumu")],
        tags=None,
        files=dict(mumu=mumu, cosmic=cosmic),
        timedep=None,
        constraints=[
            alignment.constraints.VXDHierarchyConstraints(type=2, pxd=True, svd=True),
            alignment.constraints.CDCLayerConstraints(z_offset=True, z_scale=False, twist=False)],
        fixed=alignment.parameters.vxd_sensors(layers=[3, 4, 5, 6]) +
        alignment.parameters.vxd_sensors(layers=[1, 2], rigid=False, surface2=False, surface3=False, surface4=True),
        commands=["method inversion 6 0.001", "entries 1000", "threads 10 10"],
        params=dict(minPValue=0.00001, externalIterations=0, granularity="run"),
        min_entries=80000)

    # Ignore results for BeamSpot
    std_components = ROOT.vector('string')()
    for component in ['VXDAlignment', 'CDCAlignment']:
        std_components.push_back(component)
    cal.algorithms[0].algorithm.setComponents(std_components)

    # Sequential run-by-run strategy
    cal.max_iterations = 0
    cal.algorithms[0].strategy = strategies.SequentialRunByRun

    return cal

################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):
    """
    Required function called by b2caf-prompt-run.
    Returns full configured 4-stage final alignment for prompt

    """
    seed(1234)

    cfg = kwargs['expert_config']
    files = dict()

    for colname in collection_names:
        file_to_iov = input_data[colname]
        input_files = list(file_to_iov.keys())
        files[colname] = input_files

    prompt = create_prompt(files, cfg)
    beamspot = create_beamspot(files, cfg)
    stage1 = create_stage1(files, cfg)
    stage2 = create_stage2(files, cfg)

    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    for cal in [prompt, beamspot, stage1, stage2]:
        for colname in collection_names:
            if colname not in cal.collections.keys():
                continue
            max_processed_events_per_file = cfg[f'{colname}.max_processed_events_per_file']
            basf2.set_module_parameters(
                cal.collections[colname].pre_collector_path,
                'RootInput',
                entrySequences=[f'0:{max_processed_events_per_file}'], branchNames=HLT_INPUT_OBJECTS)

        for algorithm in cal.algorithms:
            algorithm.params = {"apply_iov": output_iov}

    # Bugfix for Condor:
    for cal in [prompt, stage1, stage2]:
        from alignment.prompt_utils import fix_mille_paths_for_algo
        fix_mille_paths_for_algo(cal.algorithms[0])

    beamspot.depends_on(prompt)
    stage1.depends_on(beamspot)
    stage2.depends_on(stage1)

    return [prompt, beamspot, stage1, stage2]


if __name__ == '__main__':
    get_calibrations(dict(cosmic=dict(), mumu=dict(), hadron=dict(), offip=dict()),
                     requested_iov=IoV(0, 0, -1, -1), expert_config=default_config)
