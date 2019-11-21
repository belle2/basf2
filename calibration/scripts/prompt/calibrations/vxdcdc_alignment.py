# -*- coding: utf-8 -*-

"""VXD+CDC(layers-only) alignment with Millepede II.

Uses a mixture of input data from raw magnet ON cosmics (skim) and physics."""

from prompt import CalibrationSettings

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="VXDCDCAlignment_stage0",
                               expert_username="bilkat",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["physics", "Bcosmics"],
                               depends_on=[])

################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):

    import basf2

    # Get your input data files + IoVs separated into your input_data_names.
    file_to_iov_physics = input_data["physics"]
    file_to_iov_Bcosmics = input_data["Bcosmics"]

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, 2)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of physics files actually used as input = {len(input_files_physics)}")

    reduced_file_to_iov_Bcosmics = filter_by_max_files_per_run(file_to_iov_Bcosmics, 2)
    input_files_Bcosmics = list(reduced_file_to_iov_Bcosmics.keys())
    basf2.B2INFO(f"Total number of Bcosmics files actually used as input = {len(input_files_Bcosmics)}")

    # Get the overall IoV we want to cover for this request, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    import ROOT
    from ROOT import Belle2
    import millepede_calibration as mp2
    import alignment

    cfg = mp2.create_configuration(
        db_components=['VXDAlignment', 'CDCAlignment'],
        constraints=[
            alignment.constraints.VXDHierarchyConstraints(type=3, pxd=True, svd=True),
            alignment.constraints.CDCLayerConstraints(z_offset=True)
        ],
        fixed=alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=True),
        commands=[
            'method diagonalization 3 0.1',
            'scaleerrors 1. 1.',
            'entries 100'],
        reco_components=['PXD', 'SVD', 'CDC'],
        params=dict(minPValue=0., externalIterations=0),
        min_entries=10000)

    with cfg.reprocess_cosmics(collection_name='Bcosmics'):
        path = basf2.create_path()
        ana.fillParticleList('mu+:bad', 'z0 > 57. and abs(d0) < 26.5', path=path)
        path.add_module('SkimFilter', particleLists=['mu+:bad']).if_true(create_path())

        cfg.collect_tracks('RecoTracks', path=path)

    with cfg.reprocess_physics():
        cfg.collect_tracks('RecoTracks')

    cal = mp2.create_calibration(
        cfg,
        name='VXDCDCAlignment_stage0',
        tags=None,
        files=dict(
            physics=input_files_physics,
            Bcosmics=input_files_Bcosmics),
        timedep=[],
        init_event=(
            0,
            0,
            1003))  # init_event used to setup geometry at particular (event, run, exp) - only needed for constraint generation

    basf2.set_module_parameters(cal.collections['physics'].pre_collector_path, 'RootInput', entrySequences=['0:4000'])
    basf2.set_module_parameters(cal.collections['Bcosmics'].pre_collector_path, 'RootInput', entrySequences=['0:8000'])

    # Most values like database chain and backend args are overwritten by b2caf-prompt-run. But some can be set.
    cal.max_iterations = 3

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun so we ask you to set this up correctly.
    for algorithm in cal.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal]