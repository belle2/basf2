# -*- coding: utf-8 -*-

"""VXD+CDC(layers-only) alignment with Millepede II.

The input collections can be:
- cosmics (hlt skim) - mandatorry
- physics - all raw data -> once off-ip is available, this can be omitted
- hadron - for "low" momentum tracks from IP
- mumu - mumu_2trk or mumu_tight - for high momentum tracks from IP
- offip - not yet available - tracks from outside IP (beam background, beam-gas)

"""
import basf2
from prompt import CalibrationSettings
from prompt.calibrations.caf_cdc import settings as cdc_calibration

collection_names = ["physics", "cosmic", "hadron", "mumu", "offip"]

default_config = {
  'max_iterations': 3,
  'min_entries': 1000000,

  'method': 'diagonalization 3 0.1',
  'scaleerrors': 1.,
  'entries': 100,

  'minPValue':  0.00001,

  "physics.min_events": 400000,
  "physics.max_processed_events_per_file": 2000,

  "cosmic.min_events": 1000000,
  "cosmic.max_processed_events_per_file": 8000,

  "hadron.min_events": 400000,
  "hadron.max_processed_events_per_file": 2000,

  "mumu.min_events": 400000,
  "mumu.max_processed_events_per_file": 2000,

  "offip.min_events": 400000,
  "offip.max_processed_events_per_file": 2000,
  }

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="VXDCDCAlignment_stage0",
                               expert_username="bilkat",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=collection_names,
                               expert_config=default_config,
                               depends_on=[cdc_calibration])


def select_files(all_input_files, min_events, max_processed_events_per_file):
    from random import choice
    from prompt.utils import events_in_basf2_file
    basf2.B2INFO("Attempting to choose a good subset of files")
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
        events_contributed = 0
        if total_events_in_file < max_processed_events_per_file:
            # The file contains less than the max amount we have set (entrySequences)
            events_contributed = total_events_in_file
        else:
            events_contributed = max_processed_events_per_file
        chosen_files.append(new_file_choice)
        total_events += events_contributed

    basf2.B2INFO(f"Total chosen files = {len(chosen_files)}")
    basf2.B2INFO(f"Total events in chosen files = {total_events}")
    if total_events < min_events:
        basf2.B2FATAL(
            "There weren't enough files events selected when max_processed_events_per_file={max_processed_events_per_file}")
    return chosen_files


def create_std_path():
    import basf2
    import rawdata as raw
    import reconstruction as reco

    path = basf2.create_path()
    path.add_module('Progress')
    path.add_module('RootInput')  # , branchNames=input_branches, entrySequences=['0:5000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')
    raw.add_unpackers(path)
    path.add_module('SetupGenfitExtrapolation')
    reco.add_reconstruction(
        path,
        pruneTracks=False,
        skipGeometryAdding=True,
    )
    tmp = basf2.create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            m.param('CriticalErrorMask', 0)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path.add_module('DAFRecoFitter')
    return path


def create_cosmics_path():
    import basf2
    import rawdata as raw
    import reconstruction as reco
    import modularAnalysis as ana

    path = basf2.create_path()
    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput')  # , branchNames=input_branches, entrySequences=['0:5000'])
    path.add_module('Gearbox')
    path.add_module('Geometry')
    path.add_module(
        "TriggerSkim",
        triggerLines=["software_trigger_cut&filter&cosmic"]).if_value(
        "==0",
        basf2.Path(),
        basf2.AfterConditionPath.END)

    raw.add_unpackers(path)
    path.add_module('SetupGenfitExtrapolation')
    reco.add_cosmics_reconstruction(
        path,
        pruneTracks=False,
        skipGeometryAdding=True,
        addClusterExpertModules=False,
        data_taking_period='early_phase3',
        merge_tracks=True
    )
    tmp = basf2.create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            m.param('CriticalErrorMask', 0)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path.add_module('SetRecoTrackMomentum', automatic=True)
    path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])

    ana.fillParticleList('mu+:good', '[z0 <= 57. or abs(d0) >= 26.5] and abs(dz) > 0.4 and nTracks == 1', path=path)
    path.add_module('SkimFilter', particleLists=['mu+:good']).if_false(basf2.create_path())

    return path

################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):

    import basf2

    cfg = kwargs['expert_config']

    files = dict()

    for colname in collection_names:
        file_to_iov = input_data[colname]
        input_files = list(file_to_iov.keys())

        if not len(input_files):
            files[colname] = []
            continue

        basf2.B2INFO(f"Selecting files for: {colname}")
        input_files = select_files(input_files[:], cfg[f'{colname}.min_events'], cfg[f'{colname}.max_processed_events_per_file'])
        basf2.B2INFO(f"Total number of {colname} files actually used as input = {len(input_files)}")
        files[colname] = input_files

    # Get the overall IoV we want to cover for this request, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    from ROOT import Belle2
    import millepede_calibration as mpc
    import basf2

    import alignment.constraints
    import alignment.parameters

    print(files)

    cal = mpc.create(
        name='VXDCDCAlignment_stage0',
        dbobjects=['VXDAlignment', 'CDCAlignment'],
        collections=[
          mpc.make_collection("cosmic", path=create_cosmics_path(), tracks=["RecoTracks"]),
          mpc.make_collection("physics", path=create_std_path(), tracks=["RecoTracks"]),
          mpc.make_collection("hadron", path=create_std_path(), tracks=["RecoTracks"]),
          mpc.make_collection("mumu", path=create_std_path(), tracks=["RecoTracks"]),
          mpc.make_collection("offip", path=create_std_path(), tracks=["RecoTracks"])
          ],
        tags=None,
        files=files,
        timedep=[],
        constraints=[
            alignment.constraints.VXDHierarchyConstraints(type=2, pxd=True, svd=True),
            alignment.constraints.CDCLayerConstraints(z_offset=True, z_scale=False)
        ],
        fixed=alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=True),
        commands=[
            f"method {cfg['method']}",
            f"scaleerrors {cfg['scaleerrors']} {cfg['scaleerrors']}",
            f"entries {cfg['entries']}"],
        params=dict(minPValue=cfg['minPValue'], externalIterations=0),
        min_entries=cfg['min_entries'])

    for colname in collection_names:
        max_processed_events_per_file = cfg[f'{colname}.max_processed_events_per_file']
        basf2.set_module_parameters(
            cal.collections[colname].pre_collector_path,
            'RootInput',
            entrySequences=[f'0:{max_processed_events_per_file}'])

    # Most values like database chain and backend args are overwritten by b2caf-prompt-run. But some can be set.
    cal.max_iterations = cfg['max_iterations']

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun so we ask you to set this up correctly.
    for algorithm in cal.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal]
