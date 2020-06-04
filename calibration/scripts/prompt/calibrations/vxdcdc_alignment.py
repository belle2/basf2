# -*- coding: utf-8 -*-

"""VXD+CDC(layers-only) alignment with Millepede II.

Uses a mixture of input data from raw magnet ON cosmics (skim) and physics."""

from prompt import CalibrationSettings
from prompt.calibrations.caf_cdc import settings as cdc_calibration


#: Tells the automated system some details of this script
settings = CalibrationSettings(name="VXDCDCAlignment_stage0",
                               expert_username="bilkat",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["physics"],
                               depends_on=[cdc_calibration])


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

    ana.fillParticleList('mu+:bad', 'z0 > 57. and abs(d0) < 26.5', path=path)
    path.add_module('SkimFilter', particleLists=['mu+:bad']).if_true(basf2.create_path())

    return path

################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):

    import basf2

    # Get your input data files + IoVs separated into your input_data_names.
    file_to_iov_physics = input_data["physics"]
    file_to_iov_hlt_cosmic = input_data["physics"]

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, 1)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of physics files actually used as input = {len(input_files_physics)}")

    reduced_file_to_iov_hlt_cosmic = filter_by_max_files_per_run(file_to_iov_hlt_cosmic, 1)
    input_files_hlt_cosmic = list(reduced_file_to_iov_hlt_cosmic.keys())
    basf2.B2INFO(f"Total number of hlt_cosmic files actually used as input = {len(input_files_hlt_cosmic)}")

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

    cal = mpc.create(
        name='VXDCDCAlignment_stage0',
        dbobjects=['VXDAlignment', 'CDCAlignment'],
        collections=[
          mpc.make_collection("physics", create_std_path(), tracks=["RecoTracks"]),
          mpc.make_collection("hlt_cosmic", create_cosmics_path(), tracks=["RecoTracks"])
          ],
        tags=None,
        files=dict(
            physics=input_files_physics,
            hlt_cosmic=input_files_hlt_cosmic),
        timedep=[],
        init_event=(
            0,
            requested_iov.run_low,
            requested_iov.exp_low),
        constraints=[
            alignment.constraints.VXDHierarchyConstraints(type=2, pxd=True, svd=True),
            alignment.constraints.CDCLayerConstraints(z_offset=True, z_scale=False)
        ],
        fixed=alignment.parameters.vxd_sensors(rigid=False, surface2=False, surface3=False, surface4=True),
        commands=[
            'method diagonalization 3 0.1',
            'scaleerrors 1. 1.',
            'entries 100'],
        params=dict(minPValue=0., externalIterations=0),
        min_entries=10000)

    basf2.set_module_parameters(cal.collections['physics'].pre_collector_path, 'RootInput', entrySequences=['0:500'])
    basf2.set_module_parameters(cal.collections['hlt_cosmic'].pre_collector_path, 'RootInput', entrySequences=['0:20000'])

    # Most values like database chain and backend args are overwritten by b2caf-prompt-run. But some can be set.
    cal.max_iterations = 3

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun so we ask you to set this up correctly.
    for algorithm in cal.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal]
