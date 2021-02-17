#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
# from svd import add_svd_create_recodigits
from geometry import check_components
from analysisDQM import add_analysis_dqm, add_mirabelle_dqm


def add_common_dqm(path, components=None, dqm_environment="expressreco", dqm_mode="dont_care", create_hlt_unit_histograms=False):
    """
    This function adds DQMs which are common for Cosmic runs and Collion runs

    @param components: A list of the detector components which are available in this
                       run of basf2
    @param dqm_environment: The environment the DQM modules are running in
                            "expressreco" (default) if running on the ExpressReco system
                            "hlt" if running on the HLT online reconstructon nodes
                            If running on the hlt, you may want to output less or other DQM plots
                            due to the limited bandwith of the HLT nodes.
    @param dqm_mode: How to split up the path for online/HLT.
                     For dqm_mode == "dont_care" all the DQM modules should be added.
                     For dqm_mode == "all_events" only the DQM modules which should run on all events
                            (filtered and dismissed) should be added
                     For dqm_mode == "before_reco" only the DQM modules which should run before
                            all reconstruction
                     For dqm_mode == "filtered"  only the DQM modules which should run on filtered
                            events should be added
    @param create_hlt_unit_histograms: Parameter for SoftwareTiggerHLTDQMModule.
                                         Should be True only when running on the HLT servers
    """
    assert dqm_mode in ["dont_care", "all_events", "filtered", "before_filter"]
    # Check components.
    check_components(components)

    if dqm_environment == "expressreco" and (dqm_mode in ["dont_care"]):
        # PXD (not useful on HLT)
        if components is None or 'PXD' in components:
            path.add_module('PXDDAQDQM', histogramDirectoryName='PXDDAQ')
            path.add_module('PXDROIDQM', histogramDirectoryName='PXDROI')
            path.add_module('PXDDQMExpressReco', histogramDirectoryName='PXDER')
            path.add_module('SetupGenfitExtrapolation')
            path.add_module('PXDROIFinder',
                            recoTrackListName='RecoTracks',
                            PXDInterceptListName='PXDIntercepts')
            # moved to cosmics/collision as we need different cuts
            # path.add_module('PXDDQMEfficiency', histogramDirectoryName='PXDEFF')
            path.add_module('PXDTrackClusterDQM', histogramDirectoryName='PXDER')
            path.add_module('PXDInjectionDQM', histogramDirectoryName='PXDINJ', eachModule=True)
        # SVD
        if components is None or 'SVD' in components:
            # reconstruct SVDRecoDigits first of all
            # add_svd_create_recodigits(path)

            # SVD DATA FORMAT
            svdunpackerdqm = b2.register_module('SVDUnpackerDQM')
            path.add_module(svdunpackerdqm)
            # offline ZS emulator
            path.add_module(
                'SVDZeroSuppressionEmulator',
                SNthreshold=5,
                ShaperDigits='SVDShaperDigits',
                ShaperDigitsIN='SVDShaperDigitsZS5',
                FADCmode=True)
            # SVD Occupancy after Injection
            path.add_module('SVDDQMInjection', ShaperDigits='SVDShaperDigitsZS5')
            # SVDDQMExpressReco General
            path.add_module('SVDDQMExpressReco',
                            offlineZSShaperDigits='SVDShaperDigitsZS5')
            # SVD HIT TIME
            path.add_module('SVDDQMHitTime')
            # SVD EFFICIENCY
            path.add_module('SetupGenfitExtrapolation')
            path.add_module('SVDROIFinder',
                            recoTrackListName='RecoTracks',
                            SVDInterceptListName='SVDIntercepts')
            path.add_module('SVDDQMEfficiency')
            # SVD CLUSTERS ON TRACK
            path.add_module('SVDDQMClustersOnTrack')

        # Event time measuring detectors
        if components is None or 'CDC' in components or 'ECL' in components or 'TOP' in components:
            eventT0DQMmodule = b2.register_module('EventT0DQM')
            path.add_module(eventT0DQMmodule)

    if dqm_environment == "hlt" and (dqm_mode in ["dont_care", "before_filter"]):
        path.add_module(
            "SoftwareTriggerHLTDQM",
            createHLTUnitHistograms=create_hlt_unit_histograms,
            createTotalResultHistograms=False,
            createExpRunEventHistograms=False,
            createErrorFlagHistograms=True,
            cutResultIdentifiers={},
            histogramDirectoryName="softwaretrigger_before_filter",
        )
        path.add_module("StatisticsTimingHLTDQM",
                        createHLTUnitHistograms=create_hlt_unit_histograms,
                        )

    if dqm_environment == "hlt" and (dqm_mode in ["dont_care", "filtered"]):
        # HLT
        hlt_trigger_lines_in_plot = []
        hlt_skim_lines_in_plot = []

        hlt_trigger_lines_per_unit_in_plot = [
            "ge3_loose_tracks_inc_1_tight_not_ee2leg",
            "Elab_gt_0.5_plus_2_others_with_Elab_gt_0.18_plus_no_clust_with_Ecms_gt_2.0",
            "selectee",
            "Estargt2_GeV_cluster",
        ]
        cutResultIdentifiers = {}

        from softwaretrigger import filter_categories, skim_categories

        filter_cat = [method for method in dir(filter_categories) if method.startswith('__') is False]
        skim_cat = [method for method in dir(skim_categories) if method.startswith('__') is False]

        def read_lines(category):
            return [i.split(" ", 1)[1].replace(" ", "_") for i in category]

        for i in filter_cat:
            cutResultIdentifiers[i] = {"filter": read_lines(getattr(filter_categories, i))}
            hlt_trigger_lines_in_plot += read_lines(getattr(filter_categories, i))

        for i in skim_cat:
            cutResultIdentifiers[i] = {"skim": read_lines(getattr(skim_categories, i))}
            hlt_skim_lines_in_plot += read_lines(getattr(skim_categories, i))

        cutResultIdentifiers["skim"] = {"skim": hlt_skim_lines_in_plot}
        cutResultIdentifiers["filter"] = {"filter": hlt_trigger_lines_in_plot}

        # Default plot
        path.add_module(
            "SoftwareTriggerHLTDQM",
            cutResultIdentifiers=cutResultIdentifiers,
            l1Identifiers=["fff", "ffo", "lml0", "ffb", "fp"],
            createHLTUnitHistograms=create_hlt_unit_histograms,
            cutResultIdentifiersPerUnit=hlt_trigger_lines_per_unit_in_plot,
        )
        # Skim plots where bhabha contamination is removed
        path.add_module(
           "SoftwareTriggerHLTDQM",
           cutResultIdentifiers={
               "skim": {"skim": hlt_skim_lines_in_plot},
           },
           cutResultIdentifiersIgnored={
               "skim": [
                   "accept_bhabha_all",
                   ]
           },
           createTotalResultHistograms=False,
           createExpRunEventHistograms=False,
           histogramDirectoryName="softwaretrigger_skim_nobhabha",
        )

    if dqm_environment == "hlt" and (dqm_mode in ["dont_care", "filtered"]):
        # SVD DATA FORMAT
        if components is None or 'SVD' in components:
            svdunpackerdqm = b2.register_module('SVDUnpackerDQM')
            path.add_module(svdunpackerdqm)

    # CDC
    if (components is None or 'CDC' in components) and (dqm_mode in ["dont_care", "filtered"]):
        cdcdqm = b2.register_module('cdcDQM7')
        path.add_module(cdcdqm)

        module_names = [m.name() for m in path.modules()]
        if ('SoftwareTrigger' in module_names):
            cdcdedxdqm = b2.register_module('CDCDedxDQM')
            path.add_module(cdcdedxdqm)

        if dqm_environment == "expressreco":
            path.add_module('CDCDQM')

    # ECL
    if (components is None or 'ECL' in components) and (dqm_mode in ["dont_care", "filtered"]):
        ecldqm = b2.register_module('ECLDQM')
        path.add_module(ecldqm)
        ecldqmext = b2.register_module('ECLDQMEXTENDED')
        path.add_module(ecldqmext)
        # we dont want to create large histograms on HLT, thus ERECO only
        if dqm_environment == "expressreco":
            path.add_module('ECLDQMInjection', histogramDirectoryName='ECLINJ')
    # TOP
    if (components is None or 'TOP' in components) and (dqm_mode in ["dont_care", "filtered"]):
        topdqm = b2.register_module('TOPDQM')
        path.add_module(topdqm)
    # KLM
    if (components is None or 'KLM' in components) and (dqm_mode in ["dont_care", "filtered"]):
        klmdqm = b2.register_module("KLMDQM")
        path.add_module(klmdqm)

    # TRG before all reconstruction runs (so on all events with all unpacked information)
    if (components is None or 'TRG' in components) and (dqm_mode in ["dont_care", "before_filter"]):
        # TRGECL
        trgecldqm = b2.register_module('TRGECLDQM')
        path.add_module(trgecldqm)
        # TRGGDL
        trggdldqm = b2.register_module('TRGGDLDQM')
        trggdldqm.param('skim', 0)
        path.add_module(trggdldqm)
        # TRGTOP
        trgtopdqm = b2.register_module('TRGTOPDQM')
        trgtopdqm.param('skim', 0)
        path.add_module(trgtopdqm)
        # TRGGRL
        trggrldqm = b2.register_module('TRGGRLDQM')
        path.add_module(trggrldqm)
        # TRGCDCTSF
        nmod_tsf = [0, 1, 2, 3, 4, 5, 6]
        for mod_tsf in nmod_tsf:
            path.add_module('TRGCDCTSFDQM', TSFMOD=mod_tsf)
        # TRGCDC2D
        trgcdct2ddqm = b2.register_module('TRGCDCT2DDQM')
        path.add_module(trgcdct2ddqm)
        # TRGCDC3D
        nmod_t3d = [0, 1, 2, 3]
        for mod_t3d in nmod_t3d:
            path.add_module('TRGCDCT3DConverter',
                            hitCollectionName='FirmCDCTriggerSegmentHits' + str(mod_t3d),
                            addTSToDatastore=True,
                            EventTimeName='FirmBinnedEventT0' + str(mod_t3d),
                            addEventTimeToDatastore=True,
                            inputCollectionName='FirmTRGCDC2DFinderTracks' + str(mod_t3d),
                            add2DFinderToDatastore=True,
                            outputCollectionName='FirmTRGCDC3DFitterTracks' + str(mod_t3d),
                            add3DToDatastore=True,
                            fit3DWithTSIM=0,
                            firmwareResultCollectionName='TRGCDCT3DUnpackerStore' + str(mod_t3d),
                            isVerbose=0)
            path.add_module('TRGCDCT3DDQM', T3DMOD=mod_t3d)
        # CDCTriggerNeuro
        path.add_module('CDCTriggerNeuroDQM')
    # TRG after skim
    if (components is None or 'TRG' in components) and (dqm_mode in ["dont_care", "filtered"]):
        # TRGGDL
        trggdldqm_skim = b2.register_module('TRGGDLDQM')
        trggdldqm_skim.param('skim', 1)
        path.add_module(trggdldqm_skim)
        # TRGTOP
        trgtopdqm_skim = b2.register_module('TRGTOPDQM')
        trgtopdqm_skim.param('skim', 1)
        path.add_module(trgtopdqm_skim)

    # TrackDQM, needs at least one VXD components to be present or will crash otherwise
    if (components is None or 'SVD' in components or 'PXD' in components) and (dqm_mode in ["dont_care", "filtered"]):
        trackDqm = b2.register_module('TrackDQM')
        path.add_module(trackDqm)

    # ARICH
    if (components is None or 'ARICH' in components) and (dqm_mode in ["dont_care", "filtered"]):
        path.add_module('ARICHDQM')

    if dqm_mode in ["dont_care", "filtered"]:
        # PhysicsObjectsDQM
        add_analysis_dqm(path)
    if dqm_environment == "expressreco" and (dqm_mode in ["dont_care"]):
        add_mirabelle_dqm(path)

    # We want to see the datasize of all events after removing the raw data
    if dqm_mode in ["dont_care", "all_events"]:
        # DAQ Monitor
        path.add_module('DAQMonitor')
