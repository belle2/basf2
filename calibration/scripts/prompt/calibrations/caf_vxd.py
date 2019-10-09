# -*- coding: utf-8 -*-

"""VXD alignment with Millepede. uses a mixture of input data from raw magnet on/off cosmics and physics."""

from prompt import CalibrationSettings

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="VXD Alignment",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["physics", "cosmics", "Bcosmics"])

################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):

    import basf2

    # Get your input data files + IoVs separated into your input_data_names.
    file_to_iov_physics = input_data["physics"]
    file_to_iov_cosmics = input_data["cosmics"]
    file_to_iov_Bcosmics = input_data["Bcosmics"]

    # We might have requested an enormous amount of data across a requested range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 2

    # We filter out any more than 2 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    input_files_physics = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of physics files actually used as input = {len(input_files_physics)}")

    reduced_file_to_iov_cosmics = filter_by_max_files_per_run(file_to_iov_cosmics, max_files_per_run)
    input_files_cosmics = list(reduced_file_to_iov_cosmics.keys())
    basf2.B2INFO(f"Total number of cosmics files actually used as input = {len(input_files_cosmics)}")

    reduced_file_to_iov_Bcosmics = filter_by_max_files_per_run(file_to_iov_Bcosmics, max_files_per_run)
    input_files_Bcosmics = list(reduced_file_to_iov_Bcosmics.keys())
    basf2.B2INFO(f"Total number of Bcosmics files actually used as input = {len(input_files_Bcosmics)}")

    # Get the overall IoV we want to cover for this request, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    import ROOT
    from ROOT import Belle2
    from ROOT.Belle2 import MillepedeAlgorithm

    from caf.framework import Calibration, Collection
    from caf import strategies

    import rawdata as raw
    import reconstruction as reco
    import modularAnalysis as ana

    db_components = ['VXDAlignment']
    reco_components = None  # ['CDC', 'PXD', 'SVD']

    max_events = 10
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
        'RawKLMs']

    def MillepedeAligmentVXD(input_charged_files, input_cosmics_files, input_Bcosmics_files):

        collection_charged = Collection_Physics(input_charged_files)
        collection_cosmics = Collection_Cosmics(input_cosmics_files)
        collection_Bcosmics = Collection_BCosmics(input_Bcosmics_files)

        # Algorithm setup
        millepede = Algorithm_Millepede()

        # Calibration Setup
        calibration = Calibration('MillepedeAligmentVXD')

        # Collections setup
        # calibration.add_collection(name='cosmics', collection=collection_cosmics)
        calibration.add_collection(name='Bcosmics', collection=collection_Bcosmics)
        calibration.add_collection(name='physics', collection=collection_charged)

        calibration.algorithms = millepede

        calibration.strategies = strategies.SingleIOV  # .SequentialRunByRun

        return calibration

    def Algorithm_Millepede(components=None):
        if components is None:
            components = db_components

        millepede = MillepedeAlgorithm()

        millepede.invertSign()

        millepede.ignoreUndeterminedParams(True)

        std_components = ROOT.vector('string')()
        for component in components:
            std_components.push_back(component)
        millepede.setComponents(std_components)

        millepede.steering().command('method diagonalization 3 0.1')
        millepede.steering().command('skipemptycons')
        millepede.steering().command('threads 40 40')
        millepede.steering().command('matiter 1')
        millepede.steering().command('scaleerrors 1. 1.')
        millepede.steering().command('entries 100')
        millepede.steering().command('printcounts 1')
        # millepede.steering().command('histprint')
        millepede.steering().command('monitorresiduals')
        millepede.steering().command('closeandreopen')
        millepede.steering().command('hugecut 50.')
        millepede.steering().command('chiscut 30. 6.')
        millepede.steering().command('outlierdownweighting 3')
        millepede.steering().command('dwfractioncut 0.1')
        # millepede.steering().command('presigmas 1.')

        def fix_vxd_id(vxd_id, params=None):
            if params is None:
                params = [1, 2, 3, 4, 5, 6]
            for ipar in params:
                label = Belle2.GlobalLabel()
                label.construct(Belle2.VXDAlignment.getGlobalUniqueID(), vxd_id.getID(), ipar)
                millepede.steering().command('{} 0.0 -1. ! {}'.format(str(label.label()), str(vxd_id)))

        millepede.steering().command('Parameters')
        # Halfshells
        fix_vxd_id(Belle2.VxdID(1, 0, 0, 1))
        fix_vxd_id(Belle2.VxdID(1, 0, 0, 2))
        fix_vxd_id(Belle2.VxdID(3, 0, 0, 1))
        fix_vxd_id(Belle2.VxdID(3, 0, 0, 2))

        ladders = [8, 12, 7, 10, 12, 16]
        sensors = [2, 2, 2, 3, 4, 5]

        for layer in range(1, 7):
            for ladder in range(1, ladders[layer - 1] + 1):
                fix_vxd_id(Belle2.VxdID(layer, ladder, 0))
                for sensor in range(1, sensors[layer - 1] + 1):
                    # sensors
                    fix_vxd_id(Belle2.VxdID(layer, ladder, sensor), [20, 51, 52, 53, 54, 55])
                    pass

        return millepede

    def Path_StdReco():
        path = basf2.create_path()
        path.add_module('RootInput',
                        entrySequences=['0:{}'.format(max_events)],
                        branchNames=input_branches)
        path.add_module("Gearbox")
        path.add_module("Geometry")
        raw.add_unpackers(path, components=reco_components)

        reco.add_reconstruction(
          path,
          pruneTracks=False,
          add_muid_hits=True,
          components=reco_components
        )

        tmp = basf2.create_path()
        for m in path.modules():
            if m.name() == "PXDPostErrorChecker":
                continue

            if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
                m.set_log_level(basf2.LogLevel.ERROR)

            if m.name() == "SVDSpacePointCreator":
                m.param("MinClusterTime", -999)

            tmp.add_module(m)

        path = tmp

        path.add_module('DAFRecoFitter')
        path.add_module('AlignDQM')
        # path.add_module('TrackDQM')
        return path

    def Path_Cosmics():
        path = basf2.create_path()
        path.add_module('RootInput',
                        entrySequences=['0:{}'.format(max_events)],
                        branchNames=input_branches)
        path.add_module("Gearbox")
        path.add_module("Geometry")
        raw.add_unpackers(path, components=reco_components)
        path.add_module('SetupGenfitExtrapolation')
        reco.add_cosmics_reconstruction(
            path,
            components=reco_components,
            pruneTracks=False,
            skipGeometryAdding=True,
            addClusterExpertModules=False,
            data_taking_period='early_phase3',
            merge_tracks=True
        )

        tmp = basf2.create_path()
        for m in path.modules():
            # if m.name() == 'trgeclUnpacker':
            #    m.if_false(create_path())
            if m.name() == "PXDPostErrorChecker":
                continue

            if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
                m.set_log_level(basf2.LogLevel.ERROR)

            if m.name() == "SVDSpacePointCreator":
                m.param("MinClusterTime", -999)

            tmp.add_module(m)

        path = tmp
        path.add_module('SetRecoTrackMomentum')
        path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])
        path.add_module('AlignDQM')
        # path.add_module('TrackDQM')

        return path

    def Collection_Physics(files):
        path = Path_StdReco()

        track_variables = [
            'd0',
            'z0',
            'phi0',
            'omega',
            'tanlambda',
            'pt',
            'pionID',
            'protonID',
            'electronID',
            'muonID',
            'deuteronID',
            'nVXDHits',
            'nPXDHits',
            'nSVDHits',
            'nCDCHits',
            'nTracks']

        # two track events with vertex constraint
        ana.fillParticleList('mu+:mu_dimuon', 'abs(formula(z0)) < 0.5 and abs(d0) < 0.5 and nTracks == 2', writeOut=True, path=path)
        ana.variablesToNtuple('mu+:mu_dimuon', variables=track_variables, filename='analysis_mu_dimuon.root', path=path)
        import variables.collections
        ana.reconstructDecay('Z0:mumu -> mu-:mu_dimuon mu+:mu_dimuon', '', writeOut=True, path=path)
        ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.00, path=path)
        ana.variablesToNtuple(
            'Z0:mumu',
            variables=[
                'chiProb',
                'nTracks',
                'daughterDiffOf(0, 1, d0)',
                'daughterDiffOf(0, 1, z0)',
                'InvM'] +
            variables.collections.vertex,
            filename='analysis_dimuons.root',
            path=path)
        ana.rankByHighest(particleList='Z0:mumu', variable='chiProb', cut='nTracks == 2', numBest=1, path=path)

        ana.fillParticleList('pi+:all_charged', '', writeOut=True, path=path)
        ana.variablesToNtuple('pi+:all_charged', variables=track_variables, filename='analysis_all_charged.root', path=path)

        # all charged tracks excluding proton from beam-gas events and additional cut to remove charge asymmetry
        ana.fillParticleList(
            'pi+:charged',
            'protonID < 0.000001 and deuteronID < 0.000001 and abs(omega) > 0.002',
            writeOut=True,
            path=path)
        ana.variablesToNtuple('pi+:charged', variables=track_variables, filename='analysis_charged.root', path=path)

        # path.add_module('CopyRecoTracksWithOverlap', particleList='pi+:all_charged')
        # path.add_module('DAFRecoFitter', recoTracksStoreArrayName='RecoTracksWithOverlap')

        # collector = Collector_Millepede(vertices=['Z0:mumu'])
        collector = Collector_Millepede(tracks=['RecoTracks'])

        collection = Collection(collector=collector,
                                input_files=files,
                                pre_collector_path=path,
                                )
        return collection

    def Collection_Cosmics(files):
        path = Path_Cosmics()
        collector = Collector_Millepede(tracks=['RecoTracks'])

        collection = Collection(collector=collector,
                                input_files=files,
                                pre_collector_path=path,
                                )
        return collection

    def Collection_BCosmics(files):
        path = Path_Cosmics()
        collector = Collector_Millepede(tracks=['RecoTracks'])

        collection = Collection(collector=collector,
                                input_files=files,
                                pre_collector_path=path,
                                )
        return collection

    def Collector_Millepede(**argk):
        collector = basf2.register_module('MillepedeCollector')
        collector.param('granularity', 'run')
        collector.param('calibrateVertex', True)
        collector.param('calibrateKinematics', False)
        collector.param('minUsedCDCHitFraction', 0.8)
        collector.param('minPValue', 1.0e-5)
        collector.param('externalIterations', 0)
        collector.param('tracks', [])
        # collector.param('fitTrackT0', True)
        collector.param('components', db_components)
        collector.param('hierarchyType', 2)
        collector.param('useGblTree', False)
        collector.param('absFilePaths', True)
        # collector.param('enableWireByWireAlignment', True)
        # collector.param('enableWireSagging', True)
        collector.param(argk)

        return collector

    mp2_full = MillepedeAligmentVXD(input_files_physics, input_files_cosmics, input_files_Bcosmics)

    # Most values like database chain and backend args are overwritten by b2caf-prompt-run. But some can be set.
    mp2_full.max_iterations = 3

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun so we ask you to set this up correctly.
    for algorithm in mp2_full.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [mp2_full]
