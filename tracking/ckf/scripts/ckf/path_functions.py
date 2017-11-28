def add_pxd_ckf(path, svd_cdc_reco_tracks, pxd_reco_tracks, use_mc_truth=False, filter_cut=0.03,
                overlap_cut=0.2, use_best_seeds=10, use_best_results=2):
    """
    Convenience function to add the PXD ckf to the path.
    :param path: The path to add the module to
    :param svd_cdc_reco_tracks: The name of the already created SVD+CDC reco tracks
    :param pxd_reco_tracks: The name to output the PXD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param filter_cut: CKF parameter for MVA state filter
    :param overlap_cut: CKF parameter for MVA overlap filter
    :param use_best_results: CKF parameter for useBestNInSeed
    :param use_best_seeds: CKF parameter for UseNStates
    """

    if "PXDSpacePointCreator" not in path:
        path.add_module("PXDSpacePointCreator")

    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=svd_cdc_reco_tracks)

    if use_mc_truth:
        module_parameters = dict(
            firstHighFilter="truth",
            secondHighFilter="all",
            thirdHighFilter="all",

            filter="truth",
            useBestNInSeed=1
        )
    else:
        module_parameters = dict(
            firstHighFilterParameters={"cut": filter_cut, "identifier": "tracking/data/ckf_ToPXDStateFilter_1.xml"},
            firstHighUseNStates=use_best_seeds,

            secondHighFilterParameters={"cut": filter_cut, "identifier": "tracking/data/ckf_ToPXDStateFilter_2.xml"},
            secondHighUseNStates=use_best_seeds,

            thirdHighFilterParameters={"cut": filter_cut, "identifier": "tracking/data/ckf_ToPXDStateFilter_3.xml"},
            thirdHighUseNStates=use_best_seeds,

            filterParameters={"cut": overlap_cut, "identifier": "tracking/data/ckf_PXDTrackCombination.xml"},
            useBestNInSeed=use_best_results,
        )

    path.add_module("ToPXDCKF",
                    inputRecoTrackStoreArrayName=svd_cdc_reco_tracks,
                    outputRecoTrackStoreArrayName=pxd_reco_tracks,
                    **module_parameters)


def add_seeded_svd_ckf(path, cdc_reco_tracks, svd_reco_tracks, use_mc_truth, temporary_vxd_track_cands,
                       filter_cut=0.1):
    """
    Convenience function to add the SVD ckf to the path.
    :param path: The path to add the module to
    :param cdc_reco_tracks: The name of the already created CDC reco tracks
    :param svd_reco_tracks: The name to output the SVD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param filter_cut: CKF parameter for MVA filter
    """

    if "SVDSpacePointCreator" not in path:
        path.add_module("SVDSpacePointCreator")

    # Then, add the CKF
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks)

    if use_mc_truth:
        module_parameters = dict(
            firstHighFilter="truth",

            advanceHighFilter="advance",

            secondHighFilter="all",

            updateHighFilter="fit",

            thirdHighFilter="all",
        )

    else:
        module_parameters = dict(
            firstHighFilter="mva",
            firstHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDSeededStateFilter_1.xml", "cut": filter_cut},

            advanceHighFilter="advance",

            secondHighFilter="mva",
            secondHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDSeededStateFilter_2.xml", "cut": filter_cut},

            updateHighFilter="fit",

            thirdHighFilter="mva",
            thirdHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDSeededStateFilter_3.xml", "cut": filter_cut},
        )

    path.add_module("CDCToSVDSeedCKF",
                    minimalPtRequirement=0,

                    inputRecoTrackStoreArrayName=cdc_reco_tracks,
                    outputRecoTrackStoreArrayName=svd_reco_tracks,
                    temporaryVXDTracksStoreArrayName=temporary_vxd_track_cands,

                    hitFilter="seeded",
                    hitFilterParameters={"vxdTracksStoreArrayName": temporary_vxd_track_cands},
                    seedFilter="distance",

                    **module_parameters)


def add_svd_ckf(path, cdc_reco_tracks, svd_reco_tracks, use_mc_truth,
                filter_cut=0.1, overlap_cut=0.0, use_best_results=3, use_best_seeds=5):
    """
    Convenience function to add the SVD ckf to the path.
    :param path: The path to add the module to
    :param cdc_reco_tracks: The name of the already created CDC reco tracks
    :param svd_reco_tracks: The name to output the SVD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param filter_cut: CKF parameter for MVA filter
    :param overlap_cut: CKF parameter for MVA overlap filter
    :param use_best_results: CKF parameter for useNResults
    :param use_best_seeds: CKF parameter for useBestNInSeed
    """

    if "SVDSpacePointCreator" not in path:
        path.add_module("SVDSpacePointCreator")

    # Then, add the CKF
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks)

    if use_mc_truth:
        module_parameters = dict(
            firstHighFilter="truth",

            advanceHighFilter="advance",

            secondHighFilter="all",

            updateHighFilter="fit",

            thirdHighFilter="all",

            filter="truth",
            useBestNInSeed=1
        )
    else:
        module_parameters = dict(
            firstHighFilter="mva",
            firstHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDStateFilter_1.xml", "cut": filter_cut},
            firstHighUseNStates=use_best_seeds,

            advanceHighFilter="advance",

            secondHighFilter="mva",
            secondHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDStateFilter_2.xml", "cut": filter_cut},
            secondHighUseNStates=use_best_seeds,

            updateHighFilter="fit",

            thirdHighFilter="mva",
            thirdHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDStateFilter_3.xml", "cut": filter_cut},
            thirdHighUseNStates=use_best_seeds,

            filter="mva",
            filterParameters={"cut": overlap_cut, "identifier": "tracking/data/ckf_CDCToSVDResult.xml"},
            useBestNInSeed=use_best_results,
        )

    path.add_module("CDCToSVDSpacePointCKF",
                    minimalPtRequirement=0,
                    minimalHitRequirement=1,

                    useAssignedHits=False,

                    inputRecoTrackStoreArrayName=cdc_reco_tracks,
                    outputRecoTrackStoreArrayName=svd_reco_tracks,
                    hitFilter="sensor",
                    seedFilter="distance",

                    enableOverlapResolving=True,

                    **module_parameters)
