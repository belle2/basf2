import basf2
from iov_conditional import phase_2_conditional


def add_ckf_based_merger(path, cdc_reco_tracks, svd_reco_tracks, use_mc_truth=False, direction="backward"):
    """
    Convenience function to add the SVD track finding using VXDTF2 and the merger based on the CKF to the path.
    :param path: The path to add the module to
    :param cdc_reco_tracks: The name of the already created CDC Reco Tracks
    :param svd_reco_tracks: The name of the already created CDC Reco Tracks
    :param use_mc_truth: Use the MC information in the CKF
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    # The CDC tracks need to be fitted
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=cdc_reco_tracks)

    if use_mc_truth:
        # MC CKF needs MC matching information
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=False,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=svd_reco_tracks)

        result_filter = "truth_svd_cdc_relation"
        result_filter_parameters = {}
    else:
        result_filter = "mva_with_relations"
        result_filter_parameters = {"cut": 0.6}

    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    path.add_module("CDCToSVDSeedCKF",
                    advanceHighFilterParameters={"direction": direction},

                    fromRelationStoreArrayName=cdc_reco_tracks,
                    toRelationStoreArrayName=svd_reco_tracks,

                    writeOutDirection=direction,

                    inputRecoTrackStoreArrayName=cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=svd_reco_tracks,
                    relationCheckForDirection=direction,
                    cdcTracksStoreArrayName=cdc_reco_tracks,
                    vxdTracksStoreArrayName=svd_reco_tracks,

                    firstHighFilterParameters={"direction": direction},
                    reverseSeed=reverse_seed,

                    filter=result_filter,
                    filterParameters=result_filter_parameters
                    ).set_name(f"CDCToSVDSeedCKF_{direction}")


def add_pxd_ckf(path, *args, **kwargs):
    """Function basically calling _add_pxd_ckf_implementation for phase2 or 3 differently"""
    if "PXDSpacePointCreator" not in [m.name() for m in path.modules()]:
        path.add_module("PXDSpacePointCreator")

    phase2_path = basf2.create_path()
    _add_pxd_ckf_implementation(phase2_path, *args, phase2=True, **kwargs)
    phase3_path = basf2.create_path()
    _add_pxd_ckf_implementation(phase3_path, *args, phase2=False, **kwargs)

    phase_2_conditional(path, phase2_path=phase2_path, phase3_path=phase3_path)


def _add_pxd_ckf_implementation(path, svd_cdc_reco_tracks, pxd_reco_tracks, phase2=False, use_mc_truth=False,
                                filter_cut=0.03, overlap_cut=None, use_best_seeds=10, use_best_results=2,
                                only_use_tracks_with_svd=True, direction="backward"):
    """
    Convenience function to add the PXD ckf to the path.
    :param path: The path to add the module to
    :param svd_cdc_reco_tracks: The name of the already created SVD+CDC reco tracks
    :param pxd_reco_tracks: The name to output the PXD reco tracks to
    :param phase2: If true, use the setup for phase 2 (instead of phase 3)
    :param use_mc_truth: Use the MC information in the CKF
    :param filter_cut: CKF parameter for MVA state filter
    :param overlap_cut: CKF parameter for MVA overlap filter. Default is 0.2 for phase 3 and 0 for phase 2.
    :param use_best_results: CKF parameter for useBestNInSeed
    :param use_best_seeds: CKF parameter for UseNStates
    :param only_use_tracks_with_svd: Include a cut on the input tracks to have SVD hits
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    path.add_module("DAFRecoFitter", recoTracksStoreArrayName=svd_cdc_reco_tracks)

    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    if use_mc_truth:
        path.add_module("MCRecoTracksMatcher", UsePXDHits=False, UseSVDHits=True, UseCDCHits=True,
                        mcRecoTracksStoreArrayName="MCRecoTracks",
                        prRecoTracksStoreArrayName=svd_cdc_reco_tracks)

        module_parameters = dict(
            firstHighFilter="truth",
            secondHighFilter="all",
            thirdHighFilter="all",

            filter="truth",
            useBestNInSeed=1
        )
    else:
        if overlap_cut is None:
            if phase2:
                overlap_cut = 0.0
            else:
                overlap_cut = 0.2

        module_parameters = dict(
            firstHighFilterParameters={"cut": filter_cut, "identifier": "tracking/data/ckf_ToPXDStateFilter_1.xml",
                                       "direction": direction},
            firstHighUseNStates=use_best_seeds,

            secondHighFilterParameters={"cut": filter_cut, "identifier": "tracking/data/ckf_ToPXDStateFilter_2.xml"},
            secondHighUseNStates=use_best_seeds,

            thirdHighFilterParameters={"cut": filter_cut, "identifier": "tracking/data/ckf_ToPXDStateFilter_3.xml"},
            thirdHighUseNStates=use_best_seeds,

            filterParameters={"cut": overlap_cut, "identifier": "tracking/data/ckf_PXDTrackCombination.xml"},
            useBestNInSeed=use_best_results,
        )

    if phase2:
        module_parameters["seedHitJumping"] = 1

    path.add_module("ToPXDCKF",
                    advanceHighFilterParameters={"direction": direction},

                    writeOutDirection=direction,

                    inputRecoTrackStoreArrayName=svd_cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=pxd_reco_tracks,
                    relationCheckForDirection=direction,

                    outputRecoTrackStoreArrayName=pxd_reco_tracks,
                    outputRelationRecoTrackStoreArrayName=svd_cdc_reco_tracks,

                    onlyUseTracksWithSVD=only_use_tracks_with_svd,

                    reverseSeed=reverse_seed,
                    **module_parameters).set_name(f"ToPXDCKF_{direction}")


def add_svd_ckf(path, *args, **kwargs):
    """Function basically calling _add_svd_ckf_implementation for phase2 or 3 differently"""
    if "SVDSpacePointCreator" not in [m.name() for m in path.modules()]:
        path.add_module("SVDSpacePointCreator")

    phase2_path = basf2.create_path()
    _add_svd_ckf_implementation(phase2_path, *args, phase2=True, **kwargs)
    phase3_path = basf2.create_path()
    _add_svd_ckf_implementation(phase3_path, *args, phase2=False, **kwargs)

    phase_2_conditional(path, phase2_path=phase2_path, phase3_path=phase3_path)


def _add_svd_ckf_implementation(path, cdc_reco_tracks, svd_reco_tracks, phase2=False, use_mc_truth=False,
                                filter_cut=0.1, overlap_cut=None, use_best_results=5, use_best_seeds=10,
                                direction="backward"):
    """
    Convenience function to add the SVD ckf to the path.
    :param path: The path to add the module to
    :param cdc_reco_tracks: The name of the already created CDC reco tracks
    :param phase2: If true, use the setup for phase 2 (instead of phase 3)
    :param svd_reco_tracks: The name to output the SVD reco tracks to
    :param use_mc_truth: Use the MC information in the CKF
    :param filter_cut: CKF parameter for MVA filter
    :param overlap_cut: CKF parameter for MVA overlap filter
    :param use_best_results: CKF parameter for useNResults
    :param use_best_seeds: CKF parameter for useBestNInSeed
    :param direction: where to extrapolate to. Valid options are forward and backward
    """
    if direction == "forward":
        reverse_seed = True
    else:
        reverse_seed = False

    if overlap_cut is None:
        if phase2:
            overlap_cut = 0.0
        else:
            overlap_cut = 0.2

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
            firstHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDStateFilter_1.xml", "cut": filter_cut,
                                       "direction": direction},
            firstHighUseNStates=use_best_seeds,

            secondHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDStateFilter_2.xml", "cut": filter_cut},
            secondHighUseNStates=use_best_seeds,

            thirdHighFilterParameters={"identifier": "tracking/data/ckf_CDCSVDStateFilter_3.xml", "cut": filter_cut},
            thirdHighUseNStates=use_best_seeds,

            filterParameters={"cut": overlap_cut, "identifier": "tracking/data/ckf_CDCToSVDResult.xml"},
            useBestNInSeed=use_best_results,
        )

    if phase2:
        module_parameters["seedHitJumping"] = 3

    path.add_module("CDCToSVDSpacePointCKF",
                    inputRecoTrackStoreArrayName=cdc_reco_tracks,
                    outputRecoTrackStoreArrayName=svd_reco_tracks,
                    outputRelationRecoTrackStoreArrayName=cdc_reco_tracks,
                    relatedRecoTrackStoreArrayName=svd_reco_tracks,

                    advanceHighFilterParameters={"direction": direction},
                    reverseSeed=reverse_seed,

                    writeOutDirection=direction,
                    relationCheckForDirection=direction,

                    **module_parameters).set_name(f"CDCToSVDSpacePointCKF_{direction}")
