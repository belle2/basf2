#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from daqdqm.commondqm import add_common_dqm


def add_cosmic_dqm(path, components=None, dqm_environment="expressreco", dqm_mode="dont_care",
                   create_hlt_unit_histograms=False):
    """
    This function adds DQMs for CRT

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
                     For dqm_mode == "before_filter" only the DQM modules which should run before
                            all reconstruction
                     For dqm_mode == "filtered"  only the DQM modules which should run on filtered
                            events should be added
                     For dqm_mode == "l1_passthrough" only the DQM modules which should run on the
                            L1 passthrough events should be added
    @param create_hlt_unit_histograms: Parameter for SoftwareTiggerHLTDQMModule.
                                       Should be True only when running on the HLT servers
    """

    assert dqm_mode in ["dont_care", "all_events", "filtered", "before_filter", "l1_passthrough"]

    add_common_dqm(path, components=components, dqm_environment=dqm_environment,
                   dqm_mode=dqm_mode, create_hlt_unit_histograms=create_hlt_unit_histograms)

    if dqm_environment == "expressreco" and (dqm_mode in ["dont_care"]):
        # PXD (not useful on HLT)
        if components is None or 'PXD' in components:
            # need to be behind add_common_dqm as intercepts are calculated there
            # disable d0 and z0 cut for cosmics
            path.add_module('PXDDQMEfficiency', histogramDirectoryName='PXDEFF', z0minCut=-9999, z0maxCut=9999, d0Cut=-9999)

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

        from softwaretrigger import filter_categories

        filter_cat = [method for method in dir(filter_categories) if method.startswith('__') is False if method is not 'RESULTS']

        def read_lines(category):
            return [i.split(" ", 1)[1].replace(" ", "_") for i in category]

        for i in filter_cat:
            cutResultIdentifiers[i] = {"filter": read_lines(getattr(filter_categories, i))}
            hlt_trigger_lines_in_plot += read_lines(getattr(filter_categories, i))

        cutResultIdentifiers["filter"] = {"filter": hlt_trigger_lines_in_plot}

        additionalL1Identifiers = [
            'ffy',
            'fyo',
            'c4',
            'hie',
            'mu_b2b',
            'mu_eb2b',
            'beklm',
            'eklm2',
            'cdcklm1',
            'seklm1',
            'ieklm1',
            'ecleklm1',
            'fso',
            'fioiecl1',
            'ff30',
            'stt',
            'ioiecl1',
            'ioiecl2',
            'lml1',
            'lml2',
            'lml3',
            'lml4',
            'lml5',
            'lml6',
            'lml7',
            'lml8',
            'lml9',
            'lml10',
            'lml12',
            'lml13',
            'bhapur']

        # Default plot
        path.add_module(
            "SoftwareTriggerHLTDQM",
            cutResultIdentifiers=cutResultIdentifiers,
            l1Identifiers=["fff", "ffo", "lml0", "ffb", "fp"],
            additionalL1Identifiers=additionalL1Identifiers,
            createHLTUnitHistograms=create_hlt_unit_histograms,
            cutResultIdentifiersPerUnit=hlt_trigger_lines_per_unit_in_plot,
            pathLocation="after filter",
        )
