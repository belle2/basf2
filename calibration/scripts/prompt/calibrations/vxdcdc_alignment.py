# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""

Simultaneous Global and Local VXD and CDC (layers-only) alignment with Millepede II

The input collections can be (only single tracks currently):
- cosmics (hlt skim) - mandatorry
- physics - all raw data -> once off-ip is available, this can be omitted
- hadron - for "low" momentum tracks from IP
- mumu - mumu_2trk or mumu_tight - for high momentum tracks from IP
- offip - not yet available - tracks from outside IP (beam background, beam-gas)

Time-dependence can be (manually) configured for VXD half-shells and CDC layers.
For example to allow VXD alignment to change in run 10, 20 an 30 in experiment 12, you set:

>>> timedep_vxd : [[0, 10, 12], [0, 20, 12], [0, 30, 12]]

Note that the first run in your requested iov will be added automatically.

"""

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
  "cosmic.max_processed_events_per_file": 5000,

  "hadron.min_events": 100000,
  "hadron.max_processed_events_per_file": 1000,

  "mumu.min_events": 400000,
  "mumu.max_processed_events_per_file": 3000,

  "offip.min_events": 400000,
  "offip.max_processed_events_per_file": 2000,

  "timedep_vxd": [],
  "timedep_cdc": []
  }

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="VXD and CDC Alignment",
                               expert_username="bilkat",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=collection_names,
                               expert_config=default_config,
                               depends_on=[cdc_calibration])


def select_files(all_input_files, min_events, max_processed_events_per_file):
    import basf2
    from random import choice
    from prompt.utils import events_in_basf2_file
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
        basf2.B2FATAL(
            f"There weren't enough files events selected when max_processed_events_per_file={max_processed_events_per_file}")
    return chosen_files


def create_std_path():
    import basf2
    import rawdata as raw
    import reconstruction as reco

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
    import basf2
    import rawdata as raw
    import reconstruction as reco
    import modularAnalysis as ana

    path = basf2.create_path()
    path.add_module('Progress')
    # Remove all non-raw data to run the full reco again
    path.add_module('RootInput')
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

    path.add_module('SetRecoTrackMomentum', automatic=True)
    path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])

    ana.fillParticleList(
        'mu+:goodForVXDCDCAlignment',
        '[z0 <= 57. or abs(d0) >= 26.5] and abs(dz) > 0.4 and nTracks == 1',
        path=path)
    path.add_module('SkimFilter', particleLists=['mu+:goodForVXDCDCAlignment']).if_false(basf2.create_path())

    return path

################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):

    import basf2

    from caf.utils import IoV

    import millepede_calibration as mpc

    import alignment.constraints
    import alignment.parameters

    from random import seed
    seed(1234)

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
        files[colname] = input_files

    # Get the overall IoV we want to cover for this request, including the end values
    requested_iov = kwargs.get("requested_iov", None)
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # Pede command options
    method = cfg['method']
    scaleerrors = cfg['scaleerrors']
    entries = cfg['entries']

    timedep = []

    timedep_vxd = cfg['timedep_vxd']
    timedep_cdc = cfg['timedep_cdc']

    if len(timedep_vxd):
        slices = [(erx[0], erx[1], erx[2]) for erx in timedep_vxd] + [(0, requested_iov.run_low, requested_iov.exp_low)]
        timedep.append(
          (alignment.parameters.vxd_halfshells(), slices))
    if len(timedep_cdc):
        slices = [(erx[0], erx[1], erx[2]) for erx in timedep_cdc] + [(0, requested_iov.run_low, requested_iov.exp_low)]
        timedep.append((alignment.parameters.cdc_layers(), slices))

    cal = mpc.create(
        name='VXDCDCalignment',
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
        timedep=timedep,
        constraints=[
            alignment.constraints.VXDHierarchyConstraints(type=2, pxd=True, svd=True),
            alignment.constraints.CDCLayerConstraints(z_offset=True, z_scale=False)
        ],
        fixed=alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=True),
        commands=[
            f"method {method}",
            f"scaleerrors {scaleerrors} {scaleerrors}",
            f"entries {entries}"],
        params=dict(minPValue=cfg['minPValue'], externalIterations=0),
        min_entries=cfg['min_entries'])

    for colname in collection_names:
        max_processed_events_per_file = cfg[f'{colname}.max_processed_events_per_file']
        basf2.set_module_parameters(
            cal.collections[colname].pre_collector_path,
            'RootInput',
            entrySequences=[f'0:{max_processed_events_per_file}'])

    # Bugfix for Condor:
    from alignment.prompt_utils import fix_mille_paths_for_algo
    fix_mille_paths_for_algo(cal.algorithms[0])

    # Most values like database chain and backend args are overwritten by b2caf-prompt-run. But some can be set.
    cal.max_iterations = cfg['max_iterations']

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun so we ask you to set this up correctly.
    for algorithm in cal.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal]
